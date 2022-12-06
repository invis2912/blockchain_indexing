#include <stdio.h>
#include "rapidjson/document.h"
#include <iostream>
#include <chrono>
#include <stdint.h>
#include <map>
#include <string>
#include <sqlite3.h>
static int callback_block(void *data, int argc, char **argv, char **azColName){
	int i;
	printf("-------------------------------------------------------------------------------\n");
	for(i = 0; i<argc; i++){
		printf("\t%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("usage : %s [REST endpoint].\n For example: %s 200.json /api/blocks\n", argv[0],argv[0]);
		return 1;
	}

	sqlite3 *db;  /*sqlite3 db handler*/
	char *zErrMsg = 0;
	int rc;

	rc = sqlite3_open("chain.db", &db);	/*open database file*/
	if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return(0);
	}
	char sqlbuff[1000] = {0};
	/*check REST api parameters */
	sqlite3_exec(db, "PRAGMA main.page_size = 16384;", NULL, NULL, &zErrMsg);	//This is used for sqlite database speed increase
	sqlite3_exec(db, "PRAGMA main.cache_size=131072;", NULL, NULL, &zErrMsg);	/*cache size increase*/
	sqlite3_exec(db, "PRAGMA main.locking_mode=EXCLUSIVE;", NULL, NULL, &zErrMsg); /*locking excusive mode*/
	sqlite3_exec(db, "PRAGMA main.synchronous=OFF;", NULL, NULL, &zErrMsg); /*disable sync*/
	sqlite3_exec(db, "PRAGMA main.journal_mode=OFF;", NULL, NULL, &zErrMsg); /*disalbe jornale mode*/
	sqlite3_exec(db, "PRAGMA main.temp_store=MEMORY;", NULL, NULL, &zErrMsg);
	sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
	if(strcmp(argv[1], "/api/blocks") == 0) {  /*display all blocks*/
		const char *sql = "SELECT * from Blocks";
		rc = sqlite3_exec(db, sql, callback_block, argv[1], &zErrMsg);
	}
	else if(strncmp(argv[1], "/api/blocks?maxHeight=",22) == 0) { /*maxHeight*/
		int maxh;
		/*In this case we display all blocks that height is below maxHeight.*/
		sscanf(argv[1], "/api/blocks?maxHeight=%d", &maxh); /*get maxheight number*/
		sprintf(sqlbuff, "SELECT * from Blocks WHERE height <=%d", maxh); /*get maxHeight as Parameter*/
		rc = sqlite3_exec(db, sqlbuff, callback_block, argv[1], &zErrMsg);
	}
	else if(strncmp(argv[1],"/api/blocks/",12) == 0) {
		char buff[100];
		sscanf(argv[1], "/api/blocks/%s", buff);  /*Get paramter into buff*/
		if(strlen(buff) == 64) { /*check buffer length is 64 then it is hash*/
			sprintf(sqlbuff, "SELECT * from Blocks WHERE hash ='%s'", buff); /*make sql statement*/
			rc = sqlite3_exec(db, sqlbuff, callback_block, argv[1], &zErrMsg); /*Get sqlite exe result*/
		}
		else {
			char *trans = strstr(buff, "/transaction"); /*Get transaction string*/
			if(trans == NULL) { /*then it means it is get block by height*/
				int nh = atoi(buff); /*convert buff as integer*/
				sprintf(sqlbuff, "SELECT * from Blocks WHERE height =%d ORDER BY id DESC LIMIT 0, 1", nh); /*make sql statement*/
				rc = sqlite3_exec(db, sqlbuff, callback_block, argv[1], &zErrMsg); /*run sql query*/
			}
			else {
				trans[0] = 0;	/*erase transaction string by set nul char*/
				if(strlen(buff) == 64) { /**/
					sprintf(sqlbuff, "SELECT Transactions.txid, Transactions.hash, Transactions.version, Transactions.size, Transactions.vsize, Transactions.weight,Transactions.locktime, Transactions.hex, vin.coinbase, vin.sequence, vout.value, vout.n, vout.asm, vout.hex, vout.reqSigs, vout.type, vout.addresses FROM Transactions INNER JOIN Blocks ON Transactions.block_id=Blocks.id AND Blocks.hash='%s' INNER JOIN vin ON Transactions.id=vin.trans_id INNER JOIN vout ON Transactions.id=vout.trans_id", buff);
					rc = sqlite3_exec(db, sqlbuff, callback_block, argv[1], &zErrMsg); /*Get sqlite exe result*/				
				}
				else {
					int nh = atoi(buff); /*convert buff as integer*/
					sprintf(sqlbuff, "SELECT Transactions.txid, Transactions.hash, Transactions.version, Transactions.size, Transactions.vsize, Transactions.weight,Transactions.locktime, Transactions.hex, vin.coinbase, vin.sequence, vout.value, vout.n, vout.asm, vout.hex, vout.reqSigs, vout.type, vout.addresses FROM Transactions INNER JOIN Blocks ON Transactions.block_id=Blocks.id AND Blocks.height='%d' INNER JOIN vin ON Transactions.id=vin.trans_id INNER JOIN vout ON Transactions.id=vout.trans_id", nh);
					rc = sqlite3_exec(db, sqlbuff, callback_block, argv[1], &zErrMsg); /*Get sqlite exe result*/
				}
			}
		}
	}
	else if(strncmp(argv[1], "/api/addresses/", 15) == 0)
	{
		char buff[100] = {0}; /*buffer*/
		sscanf(argv[1], "/api/addresses/%s", buff); /*Get buffer from api string*/
		char *trans = strstr(buff,"/transactions"); /*find transactions string*/
		if(trans != NULL) { /*find transactions found*/
			trans[0] = 0; /*erase transactions string in buffer by setting null character*/
			sprintf(sqlbuff, "SELECT Transactions.txid, Transactions.hash, Transactions.version, Transactions.size, Transactions.vsize, Transactions.weight,Transactions.locktime, Transactions.hex, vin.coinbase, vin.sequence, vout.value, vout.n, vout.asm, vout.hex, vout.reqSigs, vout.type, vout.addresses FROM Transactions INNER JOIN Blocks ON Transactions.block_id=Blocks.id INNER JOIN vin ON Transactions.id=vin.trans_id INNER JOIN vout ON Transactions.id=vout.trans_id AND vout.addresses='%s'", buff); 
			rc = sqlite3_exec(db, sqlbuff, callback_block, argv[1], &zErrMsg); /*Get sqlite exe result*/
		}
		else {
			printf("REST api not correct.\n"); //if transaction stringnot found*
			//then REST api string not corrent
		}
	}
	else {
		printf("Unsupported API.\n"); //it is unsupported api
	}
	sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &zErrMsg);
	return 0;
}
