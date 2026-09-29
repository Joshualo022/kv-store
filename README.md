# lightstore-c

A persistent key-value database written from scratch in C, with a TCP server, length-prefixed framing, append-only log, and multi-client support.

## Build and run 


**Build and start**
``` 
mingw32-make
./kv_server
```

*The server listens on port 6379.* 

**Python Test (new terminal, python3, no packages req.)**
```
python -m unittest discover -s tests
```

*Tests basic commands, errors, concurrency, resizing*
## Architecture

Clients connect through port 6379, and each connection is handled by its own thread. 

Threads receive streams with 4-byte prefixes. The prefix tells the thread how many more bytes to read to get the full command. 

Commands are parsed and handled (GET, SET, DEL). When SET and DEL are run, the lock is taken, the hash table is updated accordingly, and the command is logged. When GET is run, the return value is copied before the lock is released so that other threads do not free the return value before it is sent. 

Finally a response is sent back to the client with the same 4-byte prefix protocol. 


## Design decisions

**Linear probing over chaining.** 
The hash table uses linear probing so that entries are stored in a contiguous block. Probing also checks neighboring slots which a CPU has likely cached, instead of following pointers across the heap. 

**Tombstones for deletion.** 
When an entry is deleted, a tombstone "" replaces the key. Because linear probing stops searching at NULL, entries that are stored past the deleted entry could be unreachable.

**Resizing at 0.7 load factor.** 
Linear probing suffers from clustering when full, making the search time no better than an array. Resizing ensures that inserts and searches can be done quickly. Also, inserting above the capacity will make the insert function loop forever. 

**Length-prefixed framing.** 
TCP is a byte stream with no message boundaries. A length prefix tells the server exactly how much to read and allows it to reject messages over the limit. It also allows messages containing newlines.  

**Append-only log with replay.** 
Commands are added to the log before the client receives confirmation in case the server unexpectedly crashes. When the server boots up, the log is replayed and the hash table is restored to the last state. 

**Open-once log with fflush.** 
Opening the file every time we need to log is costly, so the server opens the file just once at the start. Fflush is called after each log to make sure that the updates are written to the OS.

**Thread per client with a lock.** 
A global lock using Critical_Section protects the hash table from concurrent commands resulting in corruption. Locks are only claimed before operations directly affecting the hash table and released after for speed. During GET, the return value is copied so that the lock can be released without the concern of other threads freeing the value. Race conditions were reproduced without the lock, resulting in 17 and 10 errors on two tests. With the lock, multiple tests were run, all resulting in 0 errors. 

## Benchmarks

10,000 sequential SETs from a single client from local machine

- Earlier version with file open/close per write to log: ~1,300 ops/sec 
- Later version with open once and fflush: ~3200-4800 ops/sec 

## Known limitations and possible fixes

**No log compaction**
The log grows forever but can be fixed by implementing periodic snapshots.


**One thread per client**
This does not scale to thousands of clients. Would move to event loop or thread pooling. 


**Message limitations**
Values cannot contain spaces because of text-based payload and parsing protocol. Could be fixed by changing to full binary protocol with length prefixes for key and value arguments. 

**Single global lock**
A single global lock makes all operations serialized. Possible fixes include a read-write lock that runs GET commands in parallel and splitting the table into sections with locks per section to allow parallel writes. 

**Other**
- fflush does not guarantee persistence through power loss, could fix by adding fsync after writes at the cost of speed. 
- Windows only (Winsock), possible to add conditionals for different operating systems

## What I Learned
My server sent response messages with hardcoded lengths, and I set the length to be one short. Since TCP is a continuous stream, the newline from my messages remained in the stream and pushed the next messages down a line by one. I switched to strlen to grab the exact lengths of the message, which later motivated me to switch to length-prefixed framing. 

The *ht_insert* function did not check for duplicates when inserting. This was an easy thing to miss because inserting duplicate keys still allows for the server to get and delete. I found this issue when my log was replayed and two entries were inserted with the same key. I deleted one entry and was suprised to get a return value from GET. I fixed this by adding *get_index_from_key* into *ht_insert* to check for existing entries before inserting. This was an example of how a bug can sit dormant until two correct-looking parts interact.
