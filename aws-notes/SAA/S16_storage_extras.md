# S16 — Storage Extras: Snow Family, FSx, Storage Gateway, Transfer Family, DataSync

`[A - ENTIRE FILE]` — assistant-written scaffolding. Close this file and re-write the ★ blocks from memory before committing.

⚠ **Section number INFERRED.** Covers lectures **173–182**. Lecture **172 is unaccounted for** — S15 ended at 171. Check the course sidebar and rename if wrong.
⚠ **Stale-content lens: this is the worst section in the course for it.** See the block at the bottom before trusting any device name or capacity number.

---

## 1. Snow Family — what it is and when it's the answer

- **Offline, physical data transfer.** AWS ships you a ruggedized device, you copy data onto it, you ship it back.
- **Two job types:** data **migration** (move data in/out of AWS) and **edge computing** (process data where it's created).
- **★ THE TRIGGER PHRASE:** limited/no bandwidth, high network cost, unstable connection, remote site.
- **★ RULE OF THUMB (memorise this exact number):** if the transfer would take **more than a week** over the network, use Snow. Under a week → use the network (Direct Connect / DataSync).
- **Pay per data transfer job**, not per GB transferred.

### Device lineup as the course teaches it

| Device | Capacity | Primary use |
|---|---|---|
| **Snowcone** | 8 TB HDD | Space-constrained sites; too small for Snowball |
| **Snowcone SSD** | 14 TB SSD | Same, faster |
| **Snowball Edge Storage Optimized** | 80 TB | Bulk migration, TB→PB |
| **Snowball Edge Compute Optimized** | 42 TB HDD / 28 TB NVMe | Edge compute |
| **Snowmobile** | 100 PB per truck | Exabyte-scale, DC shutdown |

- **★ Snowcone specifics:** ~2.1 kg / 4.5 lbs. **You must supply your own power/battery and cables.** Two ways back: ship it, **or** plug it into the internet and use **DataSync** — the **DataSync agent is pre-installed** on Snowcone.
- **★ Snowmobile threshold:** use Snowmobile over Snowball when moving **more than 10 PB**. Security: temperature-controlled, GPS tracked, 24/7 video surveillance, escort.
- **Snowball Edge use cases:** large cloud migrations, data centre decommission, disaster recovery.

### The job workflow (order matters in questions)
1. Request the device in the console.
2. Install the **Snowball client** or **AWS OpsHub** on your servers.
3. Connect device, copy files.
4. Ship it back — it auto-routes to the correct AWS facility.
5. Data is loaded into an **S3 bucket**.
6. Device is **completely wiped**.

- **AWS OpsHub** = graphical desktop app to manage Snow devices (replaces CLI-only management). Unlock/configure devices, transfer files, launch and manage EC2 instances on the device, monitor metrics.

### ★★ ARCHITECTURE: Snowball into Glacier — near-guaranteed exam question
- **Snowball does NOT import directly into S3 Glacier.** No exceptions.
- **Correct answer: import to S3 first, then apply an S3 Lifecycle Policy to transition the objects to Glacier.**
- Any option offering direct Snowball→Glacier is a distractor.

---

## 2. Edge computing with Snow

- **Definition:** process data at the location where it's created, because that location has limited/no internet and limited/no compute. Examples: truck on the road, ship at sea, underground mining station.
- **Use cases:** pre-process data before shipping, machine learning at the edge, transcoding media streams.
- **Devices that do edge compute:** Snowcone/Snowcone SSD, Snowball Edge Compute Optimized, Snowball Edge Storage Optimized.

| Device | Compute |
|---|---|
| Snowcone / SSD | 2 vCPUs, 4 GB RAM, wired or wireless, USB-C power |
| Snowball Edge Compute Optimized | 104 vCPUs, 416 GiB RAM, optional GPU, clustering up to 16 nodes |
| Snowball Edge Storage Optimized | up to 40 vCPUs, 80 GiB RAM, object storage clustering |

- **★ What runs on the device:** **EC2 instances** and **AWS Lambda functions via AWS IoT Greengrass.**
- **Long-term deployment:** 1-year and 3-year discounted pricing available. If a question says "device stays on site for years", that's the long-term option, not a repeated job.

---

## 3. Amazon FSx — four third-party file systems, fully managed

**Foundation:** FSx lets you run **third-party** high-performance file systems on AWS without managing them. Contrast with **EFS**, which is AWS's own Linux/NFS file system.

### FSx for Windows File Server
- **Protocol: SMB. Filesystem: Windows NTFS.**
- Integrates with **Microsoft Active Directory**, supports **ACLs** and **user quotas**.
- **★ Can be mounted on Linux EC2 instances too** — a common trap, people assume Windows-only.
- Supports **Microsoft DFS Namespaces** (group file systems across multiple FS).
- Scale: 10s of GB/s, millions of IOPS, 100s of PB.
- **Storage: SSD** (latency-sensitive — databases, media processing, analytics) or **HDD** (broad workloads — home directories, CMS).
- **Can be Multi-AZ** for high availability.
- Accessible from on-premises over **VPN or Direct Connect**.
- **Backed up daily to S3.**

### FSx for Lustre
- **Lustre = "Linux" + "cluster".** For **High Performance Computing (HPC)**.
- Use cases: machine learning, analytics, video processing, financial modelling, genomics.
- Scale: 100s GB/s, millions of IOPS, **sub-millisecond latency**.
- **Storage: SSD** (low-latency, IOPS-intensive, small/random file ops) or **HDD** (throughput-intensive, large/sequential file ops).
- **★ Seamless S3 integration** — read S3 as a file system through FSx, and write computation results back to S3.
- Accessible from on-premises over VPN or Direct Connect.

**★★ Lustre deployment options — high-frequency exam discriminator**

| | Scratch | Persistent |
|---|---|---|
| Storage life | Temporary | Long-term |
| Replication | **None** — data lost if a file server fails | Replicated **within the same AZ** |
| Failure handling | Data gone | Failed files replaced within minutes |
| Performance | **6x burst, 200 MBps per TiB** | Baseline |
| Use when | Short-term processing, cost optimisation | Long-term processing, sensitive data |

### FSx for NetApp ONTAP
- **★ The compatibility answer.** Protocols: **NFS, SMB, AND iSCSI** — the only FSx that speaks all three.
- Works with Linux, Windows, macOS, VMware Cloud on AWS, Amazon WorkSpaces, AppStream 2.0, EC2, ECS, EKS.
- Storage **shrinks or grows automatically**.
- Features: snapshots, replication, **compression AND data de-duplication**, low cost.
- **Point-in-time instantaneous cloning** — useful for testing new workloads against production data.
- **Trigger phrase:** "move an existing ONTAP/NAS workload to AWS" or "must work across many OSes".

### FSx for OpenZFS
- **Protocol: NFS only** (v3, v4, v4.1, v4.2).
- **Trigger phrase:** "move an existing ZFS workload to AWS".
- Up to **1,000,000 IOPS with < 0.5 ms latency**.
- Snapshots, compression, low cost, **point-in-time instantaneous cloning**.
- **★★ Does NOT support data de-duplication.** This is the single cleanest discriminator between ONTAP and OpenZFS. De-dup in the question → ONTAP.

### ★ FSx one-line decision table

| Need | Answer |
|---|---|
| Windows shares, AD, SMB | FSx for **Windows File Server** |
| HPC, ML training, huge sequential throughput, S3-backed | FSx for **Lustre** |
| NFS + SMB + iSCSI, de-duplication, existing NetApp | FSx for **NetApp ONTAP** |
| NFS only, existing ZFS, extreme IOPS/low latency | FSx for **OpenZFS** |
| Plain Linux NFS, AWS-native, multi-AZ | **EFS** (not FSx) |

---

## 4. AWS Storage Gateway — the hybrid bridge

**Foundation:** lets on-premises systems use AWS storage as if it were local. **Use cases: disaster recovery, backup & restore, tiered storage, on-prem cache with low-latency access.**

| Gateway | Protocol | Backed by | Use when |
|---|---|---|---|
| **S3 File Gateway** | NFS, SMB | S3 | On-prem apps need file access to S3 objects |
| **FSx File Gateway** | SMB | FSx for Windows File Server | On-prem group shares / home directories, Windows-native |
| **Volume Gateway** | **iSCSI (block)** | S3, backed by **EBS snapshots** | Block storage, restore on-prem volumes |
| **Tape Gateway** | **iSCSI (VTL)** | S3 + Glacier | Existing physical-tape backup software |

- **★ S3 File Gateway details:** most recently used data is **cached locally**. Supports S3 Standard, Standard-IA, One Zone-IA, Intelligent-Tiering. **Cannot write directly to Glacier — use a Lifecycle Policy** (same pattern as Snowball). Bucket access via **IAM role per gateway**. SMB integrates with **Active Directory**.
- **★★ Volume Gateway — cached vs stored, memorise the split:**
  - **Cached volumes:** **primary data lives in AWS**, only frequently-accessed data cached on-prem. Low latency to recent data.
  - **Stored volumes:** **entire dataset lives on-premises**, scheduled async backups to S3 as EBS snapshots.
  - Question says "keep all data locally, back up to cloud" → **stored**. "Save on-prem storage, keep hot data local" → **cached**.
- **★ Tape Gateway:** Virtual Tape Library. The point is **you keep your existing tape-based backup software and processes** — no re-architecture.
- **★ Storage Gateway Hardware Appliance:** Storage Gateway normally needs on-prem **virtualization**. If the site has no hypervisor, buy the physical **hardware appliance** (available on amazon.com). Ships with CPU, memory, network and SSD cache. Works with File, Volume and Tape Gateway. **Trigger: small data centre / branch office with no virtualization infrastructure.**

---

## 5. AWS Transfer Family

- **Fully managed FTP-protocol front end on top of Amazon S3 or Amazon EFS.**
- **Three protocols:**
  - **FTP** — ⚠ **not encrypted.** Only supported **inside a VPC**; **cannot be publicly accessible.**
  - **FTPS** — FTP over SSL.
  - **SFTP** — FTP over SSH.
- Managed infrastructure: scalable, reliable, **highly available (multi-AZ)**.
- **Pricing: per provisioned endpoint per hour + data transferred in GB.** Note it's endpoint-hours, not purely usage — an idle endpoint still bills.
- **★ Identity:** store user credentials in the service, **or** integrate with **Microsoft Active Directory, LDAP, Okta, Amazon Cognito, or a custom provider**.
- Use cases: file sharing with partners, public datasets, CRM/ERP data feeds.
- **★ Trigger phrase:** "our partners can only use FTP/SFTP" — the answer is Transfer Family, not building an FTP server on EC2.

---

## 6. AWS DataSync

- **Move large amounts of data, online, on a schedule.**
- **Directions:**
  - On-premises / other cloud → AWS — **requires an agent** (NFS, SMB, HDFS, S3 API).
  - AWS → AWS between storage services — **no agent needed.**
- **★ Destinations:** S3 (**any storage class, including Glacier**), EFS, and **all FSx variants** (Windows, Lustre, NetApp ONTAP, OpenZFS).
- **★★ Scheduling: hourly, daily, or weekly. NOT continuous / not real-time replication.** If a question demands continuous replication, DataSync is the wrong answer.
- **★ File permissions and metadata are PRESERVED** (NFS POSIX, SMB ACLs). This is the discriminator against a plain S3 copy.
- **One agent task can use up to 10 Gbps**; a bandwidth limit can be configured so it doesn't saturate the link.
- **DataSync + Snowcone:** agent pre-installed on the device.

---

## 7. ★★ All AWS storage options compared — the summary lecture

| Service | One-line identity |
|---|---|
| **S3** | Object storage |
| **S3 Glacier** | Object archival |
| **EBS** | Network block storage, **one EC2 instance at a time** |
| **Instance Store** | Physical disk attached to the host, **highest IOPS, ephemeral** |
| **EFS** | Network file system for **Linux**, POSIX, multi-AZ |
| **FSx for Windows** | Network file system for **Windows**, SMB |
| **FSx for Lustre** | **HPC** Linux file system, S3-integrated |
| **FSx for NetApp ONTAP** | Broadest **OS/protocol compatibility**, de-dup |
| **FSx for OpenZFS** | Managed **ZFS**, extreme IOPS, no de-dup |
| **Storage Gateway** | **Hybrid** — S3/FSx File, Volume (cached/stored), Tape |
| **Transfer Family** | **FTP/FTPS/SFTP** interface onto S3 or EFS |
| **DataSync** | **Scheduled** data sync, on-prem→AWS or AWS→AWS |
| **Snow Family** | **Physical/offline** bulk migration + edge compute |
| **Databases** | Indexed and queryable, workload-specific |

---

## ★★ Exam trap list — the ten most likely question shapes

1. **Snowball → Glacier directly?** No. S3 first, then Lifecycle Policy.
2. **Transfer time under a week?** Network, not Snow.
3. **More than 10 PB?** Snowmobile over Snowball (per course content — see stale flags).
4. **Lustre scratch vs persistent?** Replication and data survival is the split, not speed.
5. **De-duplication needed?** FSx for **NetApp ONTAP**, never OpenZFS.
6. **iSCSI + NFS + SMB together?** ONTAP only.
7. **Volume Gateway cached vs stored?** Where the *full* dataset lives is the question.
8. **No hypervisor on site?** Storage Gateway **hardware appliance**.
9. **Plain FTP must be public?** Impossible — FTP is VPC-only in Transfer Family.
10. **Continuous replication?** Not DataSync. DataSync is scheduled.

**Also worth pre-loading:** EFS vs FSx for Windows (Linux/NFS vs Windows/SMB) · EBS vs Instance Store (persistent network vs ephemeral local) · Lambda at the edge on Snow runs **via IoT Greengrass**, not natively.

---

## ⚠⚠ STALE-CONTENT FLAGS — this section has the worst decay in the course

**Verified against AWS and press sources on Sept 4, 2026. These are externally checked, unlike the rest of this file.**

- **Snowmobile was retired in March 2024.** AWS removed it from the site and confirmed the service is no longer offered. The recording still teaches it.
- **Snowcone and Snowcone SSD were discontinued effective Nov 12, 2024.** Existing customers were supported to Nov 12, 2025. The entire Snowcone lecture describes a dead product.
- **The 80 TB Snowball Edge Storage Optimized, the 52-vCPU Compute Optimized, and the Compute Optimized with GPU were all discontinued Nov 12, 2024.** Current generation is **Storage Optimized 210 TB (NVMe, up to 1.5 GB/s)** and **Compute Optimized 104 vCPU / 416 GB RAM / 28 TB NVMe**.
- **AWS is discontinuing Snowball support in all commercial regions on Dec 31, 2026** — both Storage and Compute Optimized. GovCloud and ADC regions are unaffected. After that date the Snow Console becomes inaccessible.

**★ How to handle this for the exam, and this matters:**
- **Answer exam questions using the course content, not this block.** SAA-C03 question banks lag AWS reality by years. Snowcone, Snowmobile, 80 TB and the 10 PB threshold are still very likely to appear as valid options.
- **Do not "correct" a Snowmobile answer to Snowball in the exam.** If Snowmobile is an option for an exabyte scenario, it's probably the intended key.
- **This block exists so you don't repeat it in an interview or on the college project**, where being current is the whole point.

**Unverified in this file:** everything outside the block above is from my own knowledge and has not been checked against AWS documentation. The FSx capacity and IOPS figures and the DataSync 10 Gbps number are the most likely to have drifted. One confirmation pass recommended before committing.

**Possible content gap (not staleness):** if the course does not cover **AWS Transfer Family's AS2 protocol support** (added 2022) or **DataSync's support for other clouds** (Azure Blob, Google Cloud Storage, added 2023), those are missing rather than wrong.
