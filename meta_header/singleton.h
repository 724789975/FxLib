#ifndef SINGLETON_H
#define SINGLETON_H

#define DECLARE_SINGLETON(cls)\
private:\
    static cls* m_poInstance;\
public:\
    static bool CreateInstance()\
    {\
        if(NULL == m_poInstance)\
            m_poInstance = new cls;\
        return m_poInstance != NULL;\
    }\
    static cls* Instance(){ return m_poInstance; }\
    static void DestroyInstance()\
    {\
        if(m_poInstance != NULL)\
        {\
            delete m_poInstance;\
            m_poInstance = NULL;\
        }\
    }

#define IMPLEMENT_SINGLETON(cls) \
    cls* cls::m_poInstance = NULL;

#endif
