#include "dbreader.h"
#include "dbmodule.h"
#include "hiredis.h"

FxRedisReader::FxRedisReader()
:m_res(NULL), m_row(NULL), m_adwLengths(NULL)
{
}

FxRedisReader::~FxRedisReader()
{
	if (m_res != NULL)
	{
		mysql_free_result(m_res);
	}
}

UINT32 FxRedisReader::GetRecordCount()
{
    if (NULL != m_res)
    {
        return (UINT32)mysql_num_rows(m_res);
    }

    return 0;
}

UINT32 FxRedisReader::GetFieldCount()
{
    if (NULL != m_res)
    {
        return (UINT32)mysql_num_fields(m_res);
    }

    return 0;
}

bool FxRedisReader::GetFirstRecord()
{
    if (NULL == m_res)
    {
        return false;
    }

    mysql_data_seek(m_res, 0);
    return true;
}

bool FxRedisReader::GetNextRecord()
{
	if (NULL == m_res)
    {
		return false;
    }

	m_row = mysql_fetch_row(m_res);
	if(m_row != NULL)
	{
		m_adwLengths = mysql_fetch_lengths(m_res);
	}

	return (m_row != NULL);
}

const char* FxRedisReader::GetFieldValue(UINT32 dwIndex)
{
	if (NULL == m_row)
    {
		return NULL;
    }

	if (dwIndex >= GetFieldCount())
    {
		return NULL;
    }

	return m_row[dwIndex];
}

INT32 FxRedisReader::GetFieldLength(UINT32 dwIndex)
{
	if (NULL == m_row)
    {
		return 0;
    }

	if (dwIndex >= GetFieldCount())
    {
		return 0;
    }

	return m_adwLengths[dwIndex];
}

void FxRedisReader::Release()
{
	if (m_res != NULL)
    {
		mysql_free_result(m_res);
    }

	m_res = NULL;
	m_row = NULL;

    FxRedisModule::Instance()->ReleaseReader(this);
}

