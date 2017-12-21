#ifndef _VLL_H_
#define _VLL_H_

#include "global.h"
#include "helper.h"
#include "query.h"
#include "txn.h"
#include "cqueue.h"
#include <glib.h>
class txn_man;

class TxnQEntry {
public:
	TxnQEntry * prev;
	TxnQEntry * next;
	txn_man * 	txn;
	TxnType vll_txn_type ;
};

class VLLMan {
public:
	void init();
	void vllMainLoop(txn_man * next_txn, base_query * query);
	void exe_blocked();
	void mrsw();
	// 	 1: txn is blocked
	//	 2: txn is not blocked. Can run.
	//   3: txn_queue is full. 
	int beginTxn(txn_man * txn, base_query * query, TxnQEntry *& entry);
	void finishTxn(txn_man * txn, TxnQEntry * entry);
	void execute(txn_man * txn, base_query * query);
private:
	//txn queue
   TxnQEntry*  			_txn_queue;
   TxnQEntry* 			_txn_queue_tail;
	int 					_txn_queue_size;
	pthread_mutex_t 	 	_mutex_queue;
    // serial queue
   //TxnQEntry * 			_serial_queue;
   //TxnQEntry * 			_serial_queue_tail;
	int 					_serial_queue_size;
	//pthread_mutex_t 		_mutex_serial;
	Queue* _serial_queue;


	pthread_mutex_t 		_mutex;   //critical section for begin() and finish()

	GThreadPool *thread_pool;

	TxnQEntry * getQEntry();
	void returnQEntry(TxnQEntry * entry);
};

#endif
