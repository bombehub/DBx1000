#include "global.h"
#include "vll.h"
#include "txn.h"
#include "table.h"
#include "row.h"
#include "row_vll.h"
#include "ycsb_query.h"
#include "ycsb.h"
#include "wl.h"
#include "catalog.h"
#include "mem_alloc.h"
#if CC_ALG == VLL


void query_txn(gpointer txn2, gpointer data) {
	txn_man* txn = (txn_man*)txn2;
	pthread_rwlock_rdlock(&_rw_lock);
	for (int rid = 0; rid < txn->row_cnt; rid++) {
		row_t * row = txn->accesses[rid]->orig_row;
		access_t type = txn->accesses[rid]->type;
		if (type == WR) {
			char* data = row->data2;
		}
	}
	//printf("4");
	pthread_rwlock_unlock(&_rw_lock);
}

void VLLMan::init() {
	_txn_queue_size = 0;
	_txn_queue = NULL;
	_txn_queue_tail = NULL;
	_serial_queue_size = 0;
	_serial_queue = queue_init();
	//_serial_queue_tail = NULL;
	pthread_mutex_init(&_mutex_queue, NULL);
	//pthread_mutex_init(&_mutex_serial, NULL);
	pthread_mutex_init(&_mutex, NULL);
	pthread_rwlock_init(&_rw_lock,NULL);
	g_thread_init(NULL);
	thread_pool = g_thread_pool_new(query_txn, NULL, THREAD_QUERY, TRUE, NULL);
}

void VLLMan::exe_blocked() {
	while (1) {
		TxnQEntry* front = NULL;
		txn_man* front_txn = NULL;
		pthread_mutex_lock(&_mutex_queue);
		LIST_GET_HEAD(_txn_queue, _txn_queue_tail, front); // front = (null OR pop head)
		pthread_mutex_unlock(&_mutex_queue);
		//front_txn = (txn_man*)queue_dequeue(_txn_queue);
		if (front != NULL) {
			front_txn = front->txn;
			execute(front_txn, NULL);
			//printf("1");
			finishTxn(front_txn, front);
		}
	}
}
void VLLMan::mrsw() { // run multi mrsw in main.cpp
	while (1) {
		//pthread_mutex_lock(&_mutex_serial);
		TxnQEntry * front = NULL;
		txn_man* front_txn = NULL;
		//LIST_GET_HEAD(_serial_queue, _serial_queue_tail, front);
		front_txn = (txn_man*) queue_dequeue(_serial_queue);
		//printf("pop txn\n");
		if (front_txn) {
			//front_txn = front->txn;
			/*if (front_txn->rtype == RD) {
				pthread_rwlock_rdlock(&_rw_lock);
				for (int rid = 0; rid < front_txn->row_cnt; rid++) {
					row_t * row = front_txn->accesses[rid]->orig_row;
					access_t type = front_txn->accesses[rid]->type;
					if (type == WR) {
						char* data = row->data2;
					}
				}
				pthread_rwlock_unlock(&_rw_lock);
			} else if (front_txn->rtype == WR) {*/
				pthread_rwlock_wrlock(&_rw_lock);
				for (int rid = 0; rid < front_txn->row_cnt; rid++) {
					row_t * row = front_txn->accesses[rid]->orig_row;
					access_t type = front_txn->accesses[rid]->type;
					if (type == WR) {
						memcpy(row->data2, row->data, sizeof(row->data));
						front_txn->accesses[rid]->orig_row->manager->remove_access(
								type);
					}
				}
				//printf("3");
				pthread_rwlock_unlock(&_rw_lock);
			}

		//pthread_mutex_unlock(&_mutex_serial);
		/*		if (front_txn) {
		 if (front_txn->rtype == RD) {
		 for (int rid = 0; rid < front_txn->row_cnt; rid++) {
		 row_t * row = front_txn->accesses[rid]->orig_row;
		 access_t type = front_txn->accesses[rid]->type;
		 if (type == WR) {
		 char* data = row->data2;
		 }
		 }
		 pthread_rwlock_unlock(&_rw_lock);
		 } else if (front_txn->rtype == WR) {
		 for (int rid = 0; rid < front_txn->row_cnt; rid++) {
		 row_t * row = front_txn->accesses[rid]->orig_row;
		 access_t type = front_txn->accesses[rid]->type;
		 if (type == WR) {
		 memcpy(row->data2, row->data, sizeof(row->data));
		 front_txn->accesses[rid]->orig_row->manager->remove_access(
		 type);
		 }
		 }
		 pthread_rwlock_unlock(&_rw_lock);
		 }
		 }*/
	}
}

void VLLMan::vllMainLoop(txn_man * txn, base_query * query) {
	ycsb_query * m_query = (ycsb_query *) query;
	// access the indexes. This is not in the critical section
	for (int rid = 0; rid < m_query->request_cnt; rid++) {
		ycsb_request * req = &m_query->requests[rid];
		ycsb_wl * wl = (ycsb_wl *) txn->get_wl();
		int part_id = wl->key_to_part(req->key);
		INDEX * index = wl->the_index;
		itemid_t * item;
		item = txn->index_read(index, req->key, part_id);
		row_t * row = ((row_t *) item->location);
		// the following line adds the read/write sets to txn->accesses
		txn->get_row(row, req->rtype);
		int cs = row->manager->get_cs();
	}
	bool done = false;
	if (m_query->rtype == RD) {
		TxnQEntry* entry = getQEntry();
		txn->rtype = RD;
		entry->txn = txn;
		//pthread_mutex_lock(&_mutex_serial);
		//LIST_PUT_TAIL(_serial_queue, _serial_queue_tail, entry);
		//pthread_mutex_unlock(&_mutex_serial);
		//queue_enqueue(_serial_queue, txn);
		//printf("push query txn\n");

		g_thread_pool_push(thread_pool, (gpointer)txn, NULL);

	}
	if (m_query->rtype == WR) {
		txn_man * front_txn = NULL;
		uint64_t t5 = get_sys_clock();
		uint64_t tt5 = get_sys_clock() - t5;
		INC_STATS(txn->get_thd_id(), debug5, tt5);
		// _mutex will be unlocked in beginTxn()
		//pthread_mutex_unlock(&_mutex_queue);
		TxnQEntry * entry = getQEntry();
		txn->rtype = WR;
		entry->txn = txn;
		entry->next = NULL;
		int ok = beginTxn(txn, query, entry);
		if (ok == 2) {
			execute(txn, query);
			//printf("2");
			finishTxn(txn, entry);
		}
		assert(ok == 1 || ok == 2);
		done = true;
	}
	return;
}

int VLLMan::beginTxn(txn_man * txn, base_query * query, TxnQEntry *& entry) {
	pthread_mutex_lock(&_mutex);
	int ret = -1;
	if (_txn_queue_size >= TXN_QUEUE_SIZE_LIMIT)
		ret = 3;

	txn->vll_txn_type = VLL_Free;
	entry->txn = txn;
	entry->vll_txn_type = VLL_Free;
	assert(WORKLOAD == YCSB);

	for (int rid = 0; rid < txn->row_cnt; rid++) {
		access_t type = txn->accesses[rid]->type;
		if (txn->accesses[rid]->orig_row->manager->insert_access(type)) {
			txn->vll_txn_type = VLL_Blocked;
			entry->vll_txn_type = VLL_Blocked;
		}
	}

	if (txn->vll_txn_type == VLL_Blocked) {
		ret = 1;
		pthread_mutex_lock(&_mutex_queue);
		LIST_PUT_TAIL(_txn_queue, _txn_queue_tail, entry);
		pthread_mutex_unlock(&_mutex_queue);
		//queue_enqueue(_txn_queue,txn);
		block_num++;

	} else {
		ret = 2;
	}
	pthread_mutex_unlock(&_mutex);
	return ret;
}

void VLLMan::finishTxn(txn_man * txn, TxnQEntry * entry) {
	pthread_mutex_lock(&_mutex);
	for (int rid = 0; rid < txn->row_cnt; rid++) {
		access_t type = txn->accesses[rid]->type;
		if (type == RD)
			txn->accesses[rid]->orig_row->manager->remove_access(type);
	}

	TxnQEntry * entry2 = getQEntry();
	entry2->txn = txn;
	//pthread_mutex_lock(&_mutex_serial);
	//LIST_PUT_TAIL(_serial_queue, _serial_queue_tail, entry2);
	//pthread_mutex_unlock(&_mutex_serial);
	queue_enqueue(_serial_queue, txn);
	//printf("push update txn\n");
	pthread_mutex_unlock(&_mutex);
}

void VLLMan::execute(txn_man * txn, base_query * query) {
	RC rc;
	uint64_t t3 = get_sys_clock();
	ycsb_query * m_query = (ycsb_query *) query;
	ycsb_wl * wl = (ycsb_wl *) txn->get_wl();
	Catalog * schema = wl->the_table->get_schema();
	uint64_t average;
	myrand rdm;
	rdm.init(time(NULL));
	for (int rid = 0; rid < txn->row_cnt; rid++) {
		row_t * row = txn->accesses[rid]->orig_row;
		access_t type = txn->accesses[rid]->type;
		if (type == RD) {
			for (int fid = 0; fid < schema->get_field_cnt(); fid++) {
				char * data = row->get_data();
				uint64_t fval = *(uint64_t *) (&data[fid * 10]);
			}
		} else {
			assert(type == WR);
			for (int fid = 0; fid < schema->get_field_cnt(); fid++) {
				char * data = row->get_data();
				*(uint64_t *) (&data[fid * 10]) = rdm.next();
			}
		}
	}
	uint64_t tt3 = get_sys_clock() - t3;
	INC_STATS(txn->get_thd_id(), debug3, tt3);
}

TxnQEntry * VLLMan::getQEntry() {
	TxnQEntry * entry = (TxnQEntry *) mem_allocator.alloc(sizeof(TxnQEntry), 0);
	entry->prev = NULL;
	entry->next = NULL;
	entry->txn = NULL;
	return entry;
}

void VLLMan::returnQEntry(TxnQEntry * entry) {
	mem_allocator.free(entry, sizeof(TxnQEntry));
}

#endif
