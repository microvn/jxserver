#ifndef DBTABLE_H
#define DBTABLE_H
//定义支持复制、事务处理的，多索引记录的数据表类

#include "db.h"
#include <malloc.h>

#define MAX_INDEX			16					//一个数据表最多的索引数目
#define MAX_TABLE_NAME		256					//最长的表名字

typedef int (*GetIndexFunc)(DB *, const DBT *, const DBT *, DBT *);

#define ZDBTABLE_CACHE	0x01
#define ZDBTABLE_HASH	0x02

typedef struct ZCursor {
	bool bTravel;								//是否遍历
	DBC *dbcp;									//当前遍历使用的游标
	int index;									//当前的索引
	char *key;
	int key_size;
	char *data;									//返回的数据
	int size;									//数据的大小
}tagZCursor;

class ZDBTable {
	DB_TYPE open_flag;											//打开数据库的操作标志
	DB *primary_db;												//存放主键-数据的数据库
	DB *index_db[MAX_INDEX];									//存放索引-主键的数据库
	GetIndexFunc get_index_funcs[MAX_INDEX];					//从数据中获得二级key的的函数列表
	bool is_index_unique[MAX_INDEX];							//索引是否唯一
	int index_number;											//二级索引数目
	char table_name[MAX_TABLE_NAME];
	int m_nLastRet;
protected:
	char env_path[MAX_TABLE_NAME];
	DB_ENV *dbenv;												//数据库环境
	ZCursor *_search(bool bKey, const char *key_ptr, int key_size, int index);		//搜索指定记录
	bool _next(bool bKey, ZCursor *cursor);															//下一个记录
public:
	bool bStop;
	ZDBTable()
	{
		dbenv			= NULL;
		primary_db		= NULL;
		index_number	= 0;
		table_name[0]	= 0;
		env_path[0]		= 0;
		bStop			= true;
		m_nLastRet		= 0;
	}
	ZDBTable(const char *path, const char *name, int nFlag = ZDBTABLE_CACHE, BOOL bIsRelativePath = TRUE)
	{
		dbenv			= NULL;
		primary_db		= NULL;
		index_number	= 0;
		table_name[0]	= 0;
		env_path[0]		= 0;
		bStop			= true;
		m_nLastRet		= 0;

		Init(path, name, nFlag, bIsRelativePath);
	}
	bool Init(const char *path, const char *name, int nFlag = ZDBTABLE_CACHE, BOOL bIsRelativePath = TRUE);   //环境目录和数据表的名字
	virtual ~ZDBTable();
	int addIndex(GetIndexFunc func, bool isUnique = false);		//增加索引项
	bool open();												//打开数据表
	void close();												//关闭数据表
	bool commit();												//基于事务的提交，目前使用自动提交，不使用
	void set_open_flag(DB_TYPE flag) {open_flag = flag;}
	//将内存中所有的数据写到硬盘，如果备份采用复制文件的方式，备份前调用该方法保证最新的数据被备份
	bool flush() {
		if (!primary_db)
			return false;
		bool bln =  (primary_db->sync(primary_db, 0) == 0); 
		for (int i=0; i<index_number; i++)
		{
			bln = bln && (index_db[i]->sync(index_db[i], 0) == 0);
		}

		return bln;
	} 

	//基本记录操作
	bool add(const char *key_ptr, int key_size, const char *data_ptr, int data_size);
	bool remove(const char *key_ptr, int key_size);

	void closeCursor(ZCursor *cursor) {
		if(!cursor) return;
		if(cursor->bTravel) {
			free(cursor->key);
		}
		free(cursor->data);

		delete cursor;
	}
	ZCursor *first() {											//遍历数据库，得到第一条记录	
		return _search(false, NULL, 0, -1);
	}
	ZCursor *search(const char *key_ptr, int key_size, int index = -1){
		return _search(false, key_ptr, key_size, index);		//搜索指定记录
	}
	bool next(ZCursor *cursor) {											//下一个记录
		return _next(false, cursor);
	}
	ZCursor *search_key(const char *key_ptr, int key_size, int index = -1) {	//搜索指定记录，返回主键值
		return _search(true, key_ptr, key_size, index);
	}
	bool next_key(ZCursor *cursor) {															//下一个记录，返回主键值
		return _next(true, cursor);
	}
	//下面是一些维护性的操作
	void deadlock() {						//解除死锁
		dbenv->lock_detect(dbenv, 0, DB_LOCK_DEFAULT, NULL);
	}
	void removeLog();						//清除日志文件
	static void removeLog(DB_ENV* pdbEnv)
	{
		ZDBTable zdb;
		zdb.dbenv	= pdbEnv;
		zdb.removeLog();
		zdb.dbenv	= NULL;
	}
	//*********************************************************************
	// function		: log_archlist
	// parameter	: pszPath 目标路径
	// return		: void
	// comment		: 把数据库文件和日志文件复制到指定目录下，并将原来不再使用的日志文件删除。
	//*********************************************************************
	void log_archlist(LPCSTR pszPath);
	//*********************************************************************
	// function		: SnapShot
	// parameter	: pszPath 目标路径
	// parameter	: pszDatabase 数据库文件
	// return		: void
	// comment		: 把数据库文件和日志文件复制到指定目录下，并将原来不再使用的日志文件删除。
	//*********************************************************************
	void SnapShot(LPCSTR pszPath, LPCSTR pszDatabase = NULL);
	static void SnapShot(DB_ENV* pdbEnv, LPCSTR pszPath, LPCSTR pszDatabase)
	{
		ZDBTable zdb;
		zdb.dbenv	= pdbEnv;
		zdb.SnapShot(pszPath, pszDatabase);
		zdb.dbenv	= NULL;
	}
	int GetLastDBReturn()
	{
		return m_nLastRet;
	}
	bool IsOpen()
	{
		return primary_db != NULL;
	}
protected:
	bool Get(DBT* pdbKey, DBT* pdbData)
	{
		m_nLastRet	= primary_db->get(primary_db, NULL, pdbKey, pdbData, 0);
		return !m_nLastRet;
	}
	DBC* GetDBC()
	{
		DBC*	pCursor = NULL;	//访问数据库用的游标
		if (m_nLastRet = primary_db->cursor(primary_db, NULL, &pCursor, 0))
			return NULL;
		return pCursor;
	}
};

#define MAX_RETRY	16

#endif
