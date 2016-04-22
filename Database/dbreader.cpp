#include "dbreader.h"
#include "dbmodule.h"

FxMySQLReader::FxMySQLReader()
:m_res(NULL), m_row(NULL), m_adwLengths(NULL)
{
}

FxMySQLReader::~FxMySQLReader()
{
	if (m_res != NULL)
	{
		mysql_free_result(m_res);
	}
}

UINT32 FxMySQLReader::GetRecordCount()
{
    if (NULL != m_res)
    {
        return (UINT32)mysql_num_rows(m_res);
    }

    return 0;
}

UINT32 FxMySQLReader::GetFieldCount()
{
    if (NULL != m_res)
    {
        return (UINT32)mysql_num_fields(m_res);
    }

    return 0;
}

bool FxMySQLReader::GetFirstRecord()
{
    if (NULL == m_res)
    {
        return false;
    }

    mysql_data_seek(m_res, 0);
    return true;
}

bool FxMySQLReader::GetNextRecord()
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

const char* FxMySQLReader::GetFieldValue(UINT32 dwIndex)
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

INT32 FxMySQLReader::GetFieldLength(UINT32 dwIndex)
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

void FxMySQLReader::Release()
{
	if (m_res != NULL)
    {
		mysql_free_result(m_res);
    }

	m_res = NULL;
	m_row = NULL;

    FxDBModule::Instance()->ReleaseReader(this);
}

