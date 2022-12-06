# Blockchain Data Indexer

We are using json file data for indexing and fetching the data from queries.


To run indexer you have to install rapidjson.

- For Linux :
```bash
sudo apt-get update

sudo apt-get install sqlite3 libsqlite3-dev

sudo apt-get install rapidjson-dev
```

rapidjson can work GNU C++ 3.8.x on Cygwin.

After installation, run :

```bash
g++ -o main source.cpp
```
## Test cases:

```bash
./indexer 200.json

./main /api/blocks

./main /api/blocks?maxHeight=20

./main /api/blocks/198

./main /api/blocks/d744db74fb70ed42767ae028a129365fb4d7de54ba1b6575fb047490554f8a7b

./main /api/blocks/198/transactions

./main /api/blocks/d744db74fb70ed42767ae028a129365fb4d7de54ba1b6575fb047490554f8a7b/transactions

./main /api/addresses/mwsZw8nF7pKxWH8eoKL9tPxTpaFkz7QeLU
```

Here, 200.json is json database file.

Indexer makes sqlite3 database file(chain.db) from 200.json.

/api/**** is REST interface provided.


- For Windows :

https://github.com/Tencent/rapidjson

https://www.sqlite.org/2022/sqlite-amalgamation-3400000.zip

RapidJSON is a header-only C++ library.

Just copy the include/rapidjson folder to the system or project's include path. Visual C++ 2008/2010/2013 on Windows (32/64-bit).

sqlite3 is also header and source file. so you can copy it to your source directory and can use it easily.


## About The Project

In this project, I implemented a simple blockchain C++ indexer using rapidjson and sqlite3 library. 

- rapidjson is a header-only c++ library for json_file parsing.
 
The blockchain's test database file is a .json_file and it has hundreds of block strings. 

In my indexer program, I open json_file using standard C++ standard File I/O functions (fopen, fread, fclose...).

Then the indexer reads data from a json_file and inserts data to sqlite3 database. 

To speed-up sqlite insert operation, I have increased cache size and turned off synchronous and journal mode. 

Begin transaction before loop and immediately commit after it. 

In my main program, it opens sqlite database and compares with REST api string to predefined strings. 

There are 
/api/blocks, /api/blocks?maxHeight=, /api/blocks/{height}, /api/blocks/{hash}, /api/blocks/{height}/transactions, /api/addresses/{address}/transactions.  

Make the sqlite query string and get result from database. 
And finally print the result.

## Suggestions for improving this task

I have created indexer.cpp and source.cpp file. Indexer.cpp is for indexing and source.cpp for results.

To improve this task,
- One can work on restructuring the program.
- Create class and functions as required.
- Code optimization and remove all unnecessary code and variables.

The current test database is less than 1MB and can allocate and read its contents at once. 
If the database file is large, we can read it step by step and can improve speed to create sqlite databases. 

We can also use other databases and parsing libraries like mysql and (https://github.com/nlohmann/json).

Apart from this, there are solutions like GraphQL which can be used for querying data directly from Blockchain by indexing.


## *** Actions/All workflows/JSON indexer Github Action ***

Anyone can modify workflow file or Build or Test file by going to main.yml file in .github/workflow directory.

### Comments


The code has comments in indexer.cpp & source.cpp.


