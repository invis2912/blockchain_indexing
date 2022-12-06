#include <stdio.h>
#include "rapidjson/document.h"
#include <iostream>
#include <chrono>
#include <stdint.h>
#include <map>
#include <string>
#include <sqlite3.h>

using namespace rapidjson;
#define SQLITE_MAX_SQL_LENGTH 1024 * 200

char mainBuff[SQLITE_MAX_SQL_LENGTH];

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	return 0;
}
void createTables(sqlite3 *db) {
	char *zErrMsg = 0;/*Create Blocks and Transactions, vin and vout table.*/
	const char* sql1 = "CREATE TABLE Blocks("  \
		"id INT NOT NULL,"\
		"hash TEXT NOT NULL,confirmations INT NOT NULL," \
		"strippedsize  INT NOT NULL, size INT NOT NULL," \
		"weight INT, height INT, mintedBlocks INT NOT NULL, stakeModifier TEXT NOT NULL,\
		version INT, versionHex TEXT, merkleroot TEXT, nonutxo_reward REAL, nonutxo_funding REAL, time INT, \
		mediantime INT, bits TEXT, difficulty REAL, chainwork TEXT,\
		nTx INT, nextblockhash TEXT, PRIMARY KEY(id, hash) );";
	int rc = sqlite3_exec(db, sql1, callback, 0, &zErrMsg);

	const char* sql2 = "CREATE TABLE Transactions("  \
		"id INT PRIMARY KEY NOT NULL,"\
		"block_id INT NOT NULL,"\
		"txid TEXT NOT NULL, hash TEXT NOT NULL, version INT NOT NULL," \
		"size INT NOT NULL, vsize INT NOT NULL, weight INT, locktime INT, " \
		"hex TEXT );";
	rc = sqlite3_exec(db, sql2, callback, 0, &zErrMsg);

	const char* sql4 = "CREATE TABLE vin("  \
		"id INT PRIMARY KEY NOT NULL,"\
		"trans_id INT NOT NULL,"\
		"coinbase TEXT, sequence INT );";
	rc = sqlite3_exec(db, sql4, callback, 0, &zErrMsg);

	const char* sql5 = "CREATE TABLE vout("  \
		"id INT PRIMARY KEY NOT NULL,"\
		"trans_id INT NOT NULL,"\
		"value REAL, n INT,asm TEXT, hex TEXT, reqSigs INT, type TEXT, addresses TEXT );";
	rc = sqlite3_exec(db, sql5, callback, 0, &zErrMsg);

}
int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("usage : %s [json file name]\n For example: %s 200.json\n", argv[0],argv[0]);
		return 1;
	}
	FILE *fp = fopen(argv[1],"rb");	/* read json test database file.*/
	if(fp == NULL) {
		printf("Can not open json database.\n");
		return 1;
	}
	fseek(fp, 0, SEEK_END);			/*move file pointer to file end.*/
	int nFileSize = ftell(fp);		/*Get size of database file.*/
	fseek(fp, 0, SEEK_SET);			/*move file pointer to file start.*/
	char *pBuff = (char*)malloc(nFileSize + 1);	/*Allocate file buffer*/
	memset(pBuff, 0, nFileSize + 1);	/*write 0 to allocated memory*/
	fread(pBuff, 1, nFileSize, fp);		/*read test database content*/
	fclose(fp);							/*Close file*/

	memset(mainBuff, 0, SQLITE_MAX_SQL_LENGTH);

	sqlite3 *db;  /*sqlite3 db handler*/
	char *zErrMsg = 0;
	int rc;

	rc = sqlite3_open("chain.db", &db);
	if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return(0);
	}

	createTables(db);
	Document doc;						/*Json document handle*/
	doc.Parse(pBuff);					/*Parse json string*/
	int nArraySize = doc.Size();		/*Get Block count on json*/
	int i, j, k;					/*Local variables for temp*/
	const char*hash, *stakeModifier,*versionHex,*merkleroot,*bits,*chainwork,*nextblockhash;
	int confirmations, strippedsize, size, weight, height, mintedBlocks, version, time, mediantime, nTx;
	double difficulty,AnchorReward;

	const char * txhash, *txid, *txhex;
	int txversion, txsize, txvsize, txweight,txlocktime;

	const char *vin_coinbase;
	int64_t vin_sequence;

	float voutvalue,IncentiveFunding;
	int voutn;
	const char*pubasm, *pubhex,*pubtype,*addresses;
	int pubsig;
	
	int vinsize, voutsize;
	int trans=0, vins = 0, vouts = 0;
	/*check REST api parameters */

	sqlite3_exec(db, "PRAGMA main.page_size = 16384;", NULL, NULL, &zErrMsg);	//This is used for sqlite database speed increase
	sqlite3_exec(db, "PRAGMA main.cache_size=131072;", NULL, NULL, &zErrMsg);	/*cache size increase*/
	sqlite3_exec(db, "PRAGMA main.locking_mode=EXCLUSIVE;", NULL, NULL, &zErrMsg); /*locking excusive mode*/
	sqlite3_exec(db, "PRAGMA main.synchronous=OFF;", NULL, NULL, &zErrMsg); /*disable sync*/
	sqlite3_exec(db, "PRAGMA main.journal_mode=OFF;", NULL, NULL, &zErrMsg); /*disalbe jornale mode*/
	sqlite3_exec(db, "PRAGMA main.temp_store=MEMORY;", NULL, NULL, &zErrMsg);
	sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);

	for(i = 0; i < nArraySize; i++) {
		Value& results = doc.GetArray()[i];  /*Get Current Array*/
		if(results.HasMember("hash"))
			hash = results["hash"].GetString(); /*Get Block hash string.*/
		else
			hash = "";
		if(results.HasMember("confirmations")) /*not hash exist then use default.*/
			confirmations = results["confirmations"].GetInt(); /*Get Block confirmations.*/
		else
			confirmations = 0;
		if(results.HasMember("strippedsize"))
			strippedsize = results["strippedsize"].GetInt();
		else
			strippedsize = 0;
		if(results.HasMember("size"))
			size = results["size"].GetInt();  /*Get Block size.*/
		else
			size = 0;
		if(results.HasMember("weight"))
			weight = results["weight"].GetInt();  /*Get Block weight.*/
		else
			weight = -1;
		if(results.HasMember("height"))
			height = results["height"].GetInt();
		else
			height = -1;
		if(results.HasMember("mintedBlocks"))
			mintedBlocks = results["mintedBlocks"].GetInt(); /*Get Block mintedBlocks.*/
		else
			mintedBlocks = 0;
		if(results.HasMember("stakeModifier"))
			stakeModifier = results["stakeModifier"].GetString(); /*Get Block stakeModifier string.*/
		else
			stakeModifier = "";
		if(results.HasMember("version"))
			version = results["version"].GetInt(); /*Get Block version.*/
		else
			version = 0;
		if(results.HasMember("versionHex"))
			versionHex = results["versionHex"].GetString();
		else
			versionHex = "";
		if(results.HasMember("merkleroot"))
			merkleroot = results["merkleroot"].GetString(); /*Get merkle root.*/
		else
			merkleroot = "";
		if(results.HasMember("time"))
			time = results["time"].GetInt();  /*Get block time.*/
		else
			time = 0;
		if(results.HasMember("mediantime"))
			mediantime = results["mediantime"].GetInt(); /*Get mediantime time.*/
		else
			mediantime = 0;
		if(results.HasMember("bits"))
			bits = results["bits"].GetString(); /*Get bits.*/
		else
			bits = "";
		if(results.HasMember("difficulty"))
			difficulty = results["difficulty"].GetFloat();  /*Get difficulty.*/
		else
			difficulty = 0;
		if(results.HasMember("chainwork"))
			chainwork = results["chainwork"].GetString(); /*Get Block chainwork.*/
		else
			chainwork = "";
		if(results.HasMember("nTx"))
			nTx = results["nTx"].GetInt(); /*Get Block Trnsaction acount.*/
		else
			nTx = 0;
		if(results.HasMember("nextblockhash"))
			nextblockhash = results["nextblockhash"].GetString(); /*Get Block nextblockhash.*/
		else
			nextblockhash = "";
		Value &nonutxo = results["nonutxo"].GetArray()[0];
		if(nonutxo.HasMember("IncentiveFunding"))
			IncentiveFunding = nonutxo["IncentiveFunding"].GetFloat(); /*Get Block IncentiveFunding.*/
		else if(nonutxo.HasMember("Burnt"))
			IncentiveFunding = nonutxo["Burnt"].GetFloat();
		else
			IncentiveFunding = 0;
		if(nonutxo.HasMember("AnchorReward"))
			AnchorReward = nonutxo["AnchorReward"].GetFloat(); /*Get Block AnchorReward.*/
		else
			AnchorReward = 0;
		if(strlen(mainBuff) > 128 * 1024) {
			sqlite3_exec(db, mainBuff, callback, 0, &zErrMsg);  /*Process 128KB's at once*/
			memset(mainBuff, 0, SQLITE_MAX_SQL_LENGTH);
		}
		char stemp[0x1000] = {0};
		sprintf(stemp, "INSERT INTO Blocks (id,hash,confirmations,strippedsize,size,\
					   weight,height, mintedBlocks, stakeModifier, version, versionHex, merkleroot,nonutxo_reward, \
					   nonutxo_funding, time, mediantime, bits, difficulty, chainwork, nTx, \
					   nextblockhash) VALUES (%d,'%s',%d,%d,%d,%d,%d,%d,'%s',%d,'%s','%s','%f',%f,%d,%d,'%s','%f','%s',%d,'%s');", \
					   i+1, hash, confirmations, strippedsize, size, weight, height, mintedBlocks, stakeModifier, version, versionHex, merkleroot,\
					   AnchorReward, IncentiveFunding, time, mediantime, bits, difficulty, chainwork, nTx, nextblockhash);
		strcat(mainBuff, stemp); /*sql sentence*/
		//rc = sqlite3_exec(db, stemp, callback, 0, &zErrMsg);
		for(j =0; j < nTx; j++) {
			Value& trarr = results["tx"].GetArray()[j];
			if(trarr.HasMember("txid"))
				txid = trarr["txid"].GetString(); /*Get Transaction id string*/
			else
				txid = "";
			if(trarr.HasMember("hash"))
				txhash = trarr["hash"].GetString(); /*Get Transaction hash string*/
			else
				txhash = "";
			if(trarr.HasMember("version"))
				txversion = trarr["version"].GetInt(); /*Get Transaction version*/
			else
				txversion = 0;
			if(trarr.HasMember("size"))
				txsize = trarr["size"].GetInt(); /*Get Transaction size*/
			else
				txsize = 0;
			if(trarr.HasMember("vsize"))
				txvsize = trarr["vsize"].GetInt(); /*Get Transaction vsize*/
			else
				txvsize = 0;
			if(trarr.HasMember("weight"))
				txweight = trarr["weight"].GetInt(); /*Get Transaction weight*/
			else
				txweight = 0;
			if(trarr.HasMember("locktime"))
				txlocktime = trarr["locktime"].GetInt(); /*Get Transaction locktime*/
			else
				txlocktime = 0;
			if(trarr.HasMember("hex"))
				txhex = trarr["hex"].GetString(); /*Get Transaction hex*/
			else
				txhex = "";
			memset(stemp, 0, 0x1000);
			sprintf(stemp, "INSERT INTO Transactions (id,block_id,txid,hash,version,\
						   size,vsize, weight, locktime, hex) VALUES (%d,%d,'%s','%s',%d,%d,%d,%d,%d,'%s');", \
						   trans + j+1, i, txid, txhash, txversion, txsize, txvsize, txweight, txlocktime, txhex);
			strcat(mainBuff, stemp); /*string connect*/

			Value vinarr = trarr["vin"].GetArray(); /*Get vin*/
			vinsize = vinarr.Size();
			for(k = 0; k < vinsize; k++) {
				Value & onevin = vinarr[k];
				if(onevin.HasMember("coinbase")) //if vin has coinbase
					vin_coinbase = onevin["coinbase"].GetString(); //get coinbase string
				else
					vin_coinbase = "";//if coinbase field not exist then fill it blank
				if(onevin.HasMember("sequence"))
					vin_sequence = onevin["sequence"].GetInt64(); /*Get vin sequence*/
				else
					vin_sequence = 0;
				memset(stemp, 0, 0x1000);
				sprintf(stemp, "INSERT INTO vin (id,trans_id,coinbase,sequence) VALUES (%d,%d,'%s',%d);", \
							   vins + k+1, trans + j, vin_coinbase, vin_sequence);
				strcat(mainBuff, stemp); /*connect sql statement*/
			}
			vins += vinsize;
			Value vinout = trarr["vout"].GetArray(); /*Get vout array*/
			voutsize = vinout.Size(); /*vout array size*/
			for(k = 0; k < voutsize; k++) {
				Value & onevout = vinout[k];
				if(onevout.HasMember("value"))
					voutvalue = onevout["value"].GetFloat(); /*Get vout*/
				else
					voutvalue = 0;
				if(onevout.HasMember("n"))
					voutn = onevout["n"].GetInt(); /*Get vout n value*/
				else
					voutn = 0;

				Value vscript = onevout["scriptPubKey"].GetObject();
				if(vscript.HasMember("asm"))
					pubasm = vscript["asm"].GetString();  /*Get vout script pubkey asm*/
				else
					pubasm = "";
				if(vscript.HasMember("hex"))
					pubhex = vscript["hex"].GetString();  /*Get vout script pubkey hex*/
				else
					pubhex = "";
				if(vscript.HasMember("reqSigs"))
					pubsig = vscript["reqSigs"].GetInt();  /*Get vout script pubkey reqsig*/
				else
					pubsig = 0;
				if(vscript.HasMember("type"))
					pubtype = vscript["type"].GetString();  /*Get vout script pubkey type*/
				else
					pubtype = "";

				if(vscript.HasMember("addresses"))
					addresses = vscript["addresses"].GetArray()[0].GetString();  /*Get vout script pubkey addresses*/
				else
					addresses = "";
				memset(stemp, 0, 0x1000);
				sprintf(stemp, "INSERT INTO vout (id,trans_id,value,n,asm,hex,reqSigs,type,addresses) VALUES (%d,%d,%f,%d,'%s','%s',%d,'%s','%s');", \
					vouts+k+1, trans+j, voutvalue, voutn, pubasm, pubhex, pubsig, pubtype,addresses);
				strcat(mainBuff, stemp);  /*string append*/
			}
			vouts += voutsize;  /*this is used or key unique*/
		}
		trans +=nTx; /*this is used or key unique*/
	}
	if(strlen(mainBuff) > 0)
		sqlite3_exec(db, mainBuff, callback, 0, &zErrMsg);
	sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &zErrMsg);
	sqlite3_close(db);
	free(pBuff);
	printf("Finished.\n");
	return 0;
}
