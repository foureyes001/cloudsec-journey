# Databases in AWS — lectures 230–239 `[A - ENTIRE FILE]`

> Summary section: maps workloads to the right database. Exam questions here are "pick the service" questions.

## 230 Choosing the right database

**Questions to ask**

| Area | Ask |
|---|---|
| Workload | Read-heavy, write-heavy or balanced? Throughput? Will it scale or fluctuate during the day? |
| Data | How much, how long kept, will it grow? Average object size? How is it accessed? |
| Durability | Is this the source of truth? |
| Latency | Latency requirement? How many concurrent users? |
| Model | How will you query it? Joins? Structured or semi-structured? |
| Schema | Strong schema, or flexibility? Reporting? Search? RDBMS or NoSQL? |
| Cost | License costs? Move to cloud-native (Aurora)? |

**Database types → service**

| Type | Service(s) | Pick when |
|---|---|---|
| Relational (SQL, OLTP) | RDS, Aurora | Joins, transactions, structured data |
| NoSQL (no joins, no SQL) | DynamoDB (JSON-like), ElastiCache (key/value), Neptune (graph), DocumentDB (MongoDB), Keyspaces (Cassandra) | Flexible schema, massive scale |
| Object store | S3 (big objects), Glacier (backups, archives) | Files, blobs |
| Data warehouse (SQL analytics, BI, OLAP) | Redshift, Athena, EMR | Analytics over large datasets |
| Search | OpenSearch | Free-text, unstructured search |
| Graph | Neptune | Relationships between data |
| Ledger | QLDB | Immutable, verifiable history (retired, see stale table) |
| Time series | Timestream | Data points over time |

## 231–239 Service cards

| Service | What it is | Scaling / HA | Key features | Use case |
|---|---|---|---|---|
| **RDS** | Managed PostgreSQL, MySQL, MariaDB, Oracle, SQL Server, Db2, + RDS Custom | You pick instance size + EBS type/size; storage auto-scaling; Read Replicas; Multi-AZ | IAM, security groups, KMS at rest, SSL in transit · automated backups with PITR up to 35 days · manual snapshots for long-term · maintenance has downtime · IAM auth · Secrets Manager integration · RDS Custom = access to the underlying instance (Oracle, SQL Server) | Relational data (OLTP), SQL queries, transactions |
| **Aurora** | AWS-built, PostgreSQL/MySQL-compatible; storage and compute separated | Storage: 6 copies across 3 AZs, self-healing, auto-growing · Compute: cluster across AZs, auto-scaling read replicas | Writer and reader endpoints, custom endpoints · same security/backup/maintenance as RDS · **Serverless** (unpredictable/intermittent load) · **Global** (cross-region, storage replication < 1 s) · **Machine Learning** (SageMaker, Comprehend) · **Cloning** (new cluster from existing, faster than snapshot restore) | Same as RDS, with less maintenance and more performance/features |
| **ElastiCache** | Managed Redis / Memcached | Cluster mode (Redis), Multi-AZ, read replicas (sharding) | In-memory, sub-millisecond latency · IAM, security groups, KMS, Redis AUTH · backup/snapshot/PITR (Redis) · **needs application code changes** | Key/value cache, frequent reads + few writes, cache DB query results, session store. **No SQL** |
| **DynamoDB** | AWS-proprietary serverless NoSQL | Multi-AZ by default; provisioned (optional auto-scaling) or on-demand capacity | Millisecond latency · max item 400 KB · transactions · **DAX** = read cache, microsecond latency · TTL · **Streams** → Lambda, or Kinesis Data Streams · **Global Tables** = active-active multi-region · PITR up to 35 days (restores to a new table) or on-demand backups · export to S3 / import from S3 without using capacity units · IAM for auth | Serverless apps with small documents, distributed serverless cache, fast-changing schemas |
| **S3** | Key/value object store | Serverless, scales without limit, max object 5 TB | Great for big objects, poor for many small ones · storage classes + lifecycle · versioning, replication, MFA-Delete, access logs · IAM, bucket policies, ACLs, Access Points, Object Lambda, CORS, Object Lock / Vault Lock · SSE-S3, SSE-KMS, SSE-C, client-side, TLS in transit · S3 Batch, S3 Inventory · multipart upload, Transfer Acceleration, S3 Select · Event Notifications → SNS, SQS, Lambda, EventBridge | Static files, big-file key/value store, static website hosting |
| **DocumentDB** | "Aurora for MongoDB": MongoDB-compatible NoSQL | Replication across 3 AZs; storage grows in 10 GB steps | Stores, queries and indexes JSON · fully managed · scales to millions of requests/s | **Any question that says MongoDB** |
| **Neptune** | Managed graph database | Replication across 3 AZs, up to 15 read replicas | Billions of relationships, millisecond queries on highly connected data · **Neptune Streams**: ordered change log of every graph change, no duplicates, read via HTTP REST API | Social networks, knowledge graphs (e.g. Wikipedia), fraud detection, recommendation engines |
| **Keyspaces** | Managed **Apache Cassandra**-compatible NoSQL | Serverless; tables replicated 3× across AZs; auto-scales tables | Uses **CQL** (Cassandra Query Language) · single-digit ms latency, 1000s req/s · on-demand or provisioned + auto-scaling · encryption, backup, PITR up to 35 days | IoT device data, time-series data. **Any question that says Cassandra** |
| **Timestream** | Serverless time-series database | Auto-scales capacity | Trillions of events/day · far faster and cheaper than relational for time series · SQL-compatible, scheduled queries, multi-measure records · recent data in memory, history in cost-optimised storage · built-in time-series analytics · encrypted in transit and at rest | IoT, operational apps, real-time analytics |

## ★ Exam keyword → service

| Question says | Answer |
|---|---|
| Relational + less ops / more performance | Aurora |
| Relational + specific engine (Oracle, SQL Server) or needs OS access | RDS (RDS Custom for OS access) |
| Unpredictable / intermittent relational load | Aurora Serverless |
| Relational across regions, < 1 s replication, DR | Aurora Global |
| New DB copy quickly from existing (e.g. staging) | Aurora Cloning |
| Cache, sub-ms, session store, needs code changes | ElastiCache |
| Serverless key/value, ms latency | DynamoDB |
| DynamoDB but microseconds | DAX |
| Multi-region active-active NoSQL | DynamoDB Global Tables |
| React to DB changes with Lambda | DynamoDB Streams |
| MongoDB | DocumentDB |
| Cassandra / CQL | Keyspaces |
| Graph, relationships, social network, fraud | Neptune |
| Time series, IoT metrics | Timestream |
| Free-text search | OpenSearch |
| Analytics / BI / OLAP | Redshift |
| Big files, not a database | S3 |

## ★ Traps
- **ElastiCache needs code changes**; DAX works with DynamoDB without rewriting queries.
- **DynamoDB PITR restores to a new table**, not in place.
- **S3 is poor for many small objects**; DynamoDB handles small items (≤ 400 KB).
- **RDS maintenance means downtime**; Aurora storage is self-healing.
- **Aurora storage = 6 copies across 3 AZs**; compute is a separate layer.

## Stale-content flags (answer the exam per the course; know reality for interviews)

| Course says | Reality now |
|---|---|
| QLDB is the ledger database | QLDB support ended July 31, 2025; AWS points users to Aurora PostgreSQL |
| Timestream | Timestream for LiveAnalytics closed to new customers June 20, 2025; AWS recommends Timestream for InfluxDB |
| Aurora Serverless | Serverless v1 reached end of life; v2 is the only option |
| S3 Select | No longer available to new customers |
| ElastiCache = Redis / Memcached | Also supports Valkey, and has a serverless option |
