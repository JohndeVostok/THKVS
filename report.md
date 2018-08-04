# THKVS: A distributed key-value storage

分布式系统结题报告

马子轩 吴华骏 余欣健

## 背景

随着互联网的不断发展, 如何管理不断增长的大量数据. 成为了非常棘手的问题. 而层出不穷的各类应用, 也对数据库系统提出了更高的要求, 尤其是容量和读写性能. 而传统的关系型数据库在这个环境中, 其可用性和可扩展性都有很大的限制. 因此, 有了NoSQL数据库的发展, 如HBASE, mongoDB, Cassandra等. 这些数据库通过对一致性的一定程度的限制, 实现了分区容错性. 从而提供更适合对应场景的服务. 在这个背景下, 我们的选题是实现一个分布式的key-value数据库. 一方面能够更多的了解目前NoSQL数据库的发展. 另一方面能够锻炼搭建系统的能力.

## 设计目标

我们的设计要求支持两种基本操作:

put(key, value): 覆盖地写入一个key, value对. 并返回操作状态.

get(key): 拿到之前写入的key对应的value.

我们的系统实现要求:

高的写入性能

动态环境中的分片与备份

一定的容错能力



## 相关工作

### CAP定理[1]

CAP定理是指一致性(consistency), 可用性(availability), 分区容错性(partition tolerance) 三个属性不能同时满足, 而传统的关系型数据库通常满足C和A两项属性. 而分布式存储通常满足A和P两条.



### 一致性hash[2]

一致性hash是一种用来解决动态环境中分片与备份的一种算法. 每台机器会分配若干个全局一致的hash值. 在进行put操作的时候, 根据key的hash值, 选取其后继的若干个不同服务器进行存储. 这种结构的好处是. 动态添加节点和删除节点的时候只会涉及到相关服务器上的数据. 其他数据的位置不会有影响. 因此广泛的使用在分布式系统中.



## 架构设计

### 整体架构

我们提供了一个P2P的无master的结构. 主要角色分为两类: client和server. 其中server会通过选举得到一个primary server.



### 一致性模型 

针对put和get操作. 我们规定一份数据备份N份, put操作返回W次成功的时候我们就记录为成功. get操作返回R次成功的时候我们就记录为成功. 显然我们应该有W + R > N. 另外, 每条数据保持一个时间戳记录写入时间. 在出现冲突的时候, 根据时间戳选取最新的数据进行读取.

client提交操作请求到任何一个server上. 这台机器通过一致性hash算法来得知操作应该作用于哪些服务器. 然后转发put, get请求到对应的服务器上. 直到返回的成功个数满足要求为止. 这样能够通过设置合适的N, W, R提供针对短时间失败的容错性. 比如我们取N = 3, W = R = 2. 一个节点宕机的时候. 对他的读写操作是失败的. 但是其他两个备份能够返回正确的状态. 此时能够继续保持服务.



### 容错

短时间容错我们的一致性模型可以提供, 针对长时间的失败, 我们采用心跳的方式动态探测所有节点的状态. 如果一个节点长时间不可用. 我们判定为他死掉了, 进行remove操作. 修复好可以再执行add操作添加回来.

add/remove会找primary进行数据迁移操作. primary拿到所有需要迁移数据的范围和起点终点. 执行数据迁移操作. 所有操作完成后. 这时候暂停所有服务. 开始修改所有主机的主机列表. 修改完成后. 再重新提供所有服务.



## 数据模型

我们在实现过程中实现了多种数据存储方式.

内存版本: 最早的版本使用了unordered_map. 但是这种无序存储无法支持数据迁移. 因此内存版本最终使用map.

list of file版本: 内存版本无法实现数据持久化. 因此采用分块方式存储数据. 块内块间保持有序. 从而支持这些操作.

b+tree: 分块方式I/O过多. b+tree能很好的减少I/O操作次数.

log-commit: 为了进一步减少I/O, 我们采取log-commit的方式存储数据. 每次操作先写log. 顺序的在文件末尾追加log信息. 一段时间之后进行commit. 能够减少文件操作次数.



## 实现细节

我们的项目使用C++实现

主要分为3个模块. driver, handler, data

driver: 负责分配操作等.

handler: 负责处理网络消息.

data: 负责存储数据.

data1个线程, driver和handler针对机器实际情况动态建立.



## 测试

测试分为功能测试和性能测试两部分.

功能测试针对put/get的正确性和节点宕机时候数据正确性. 以及动态增加删除节点时候数据迁移的正确性. 这一部分测试成功.

性能测试测试了系统的吞吐量. 测试方法是针对不同长度的一定数量的put/get操作进行计时. 

测试环境

mzxhadoop集群

mzxhadoop0:

CPU: Intel(R) Xeon(R) CPU E5-2680 v4 @ 2.40GHz

Memory: 128GB

mzxhadoop1, mzxhadoop2:

CPU: Intel(R) Xeon(R) CPU E5-2699 v4 @ 2.20GHz

Memory: 128GB

测试结果

| write(1) | write(10) | write(100) | write(1000) | write(10000) | read  |
| -------- | --------- | ---------- | ----------- | ------------ | ----- |
| 123.9    | 118.4     | 118.5      | 118.5       | 117.0        | 449.4 |

分析. 我们可以明显的看出. 写的性能和长度无关. 因此性能瓶颈一定不在网络传输和硬盘读写内容长短上. 因此我们推测性能瓶颈在大量文件开关操作上.

另外, 由于集群限制. 我们没能测试可扩展性. 但是我们的架构设计应该满足水平可扩展性.



## 后续工作

我们的工作开源在github上: https://github.com/JohndeVostok/THKVS

数据模型优化: 包括B+树和log-commit方式的优化. 以及多线程操作数据.

容错优化: 目前还有一些极特殊情况没有进行处理, 这些情况有待发掘以及解决. 以及集群状态监控等.

代码优化: 有很多冗余的锁. 还有一部分参数传递性能还有问题.



## 收获与展望

其实一开始的时候, 我们还在使用什么语言上有很大的分歧. 我主张使用golang, 而余欣健同学主张使用C++. 最后我们讨论的结果就是使用C++. 整个项目的过程中, 我们在C++的使用上踩了不少坑. 但是相对的, 在余欣健的帮助下. 我们两位的C++代码水平也有了很大的提高. 最终收获了更多.

另外, 这个项目中多线程, 多机编程的经验. 我们是很缺乏的. 在这个项目中算是都体验了一下.

最后就是系统经验, 从0开始建立一个系统, 其中坑很多, 情况很复杂. 最终我们成功的把系统跑起来, 虽然性能不够强, 而且还有没解决的问题. 但是最后还是有很大的成就感的.

回忆整个过程, 从开题报告的时候, 向老师请教各种问题, 到大家开始各自实现自己的功能, 再到后来大家集中工作. 队友们都奉献了自己宝贵的时间, 记得最后两天还有几个问题没解决的时候. 大家都自觉地加班加点工作. 最后48小时可以说一直有人在干. 真的很感谢各位.



## 参考文献

[1] Lynch, Nancy, and Seth Gilbert. “Brewer’s conjecture and the feasibility of consistent, available, partition-tolerant web services.” ACM SIGACT News, v. 33 issue 2, 2002, p. 51-59.

[2] D. Darger, E. Lehman, T. Leighton, M. Levine, D. Lewin and R. Panigrahy. Consistent Hashing and Random Trees:Distributed Caching Protocols for Relieving Hot Spots On the World Wide Web. ACM Symposium on Theory of Computing, 1997. 1997：654-663.

[3]  Giuseppe de Candia, Deniz Hastorun, Madan Jampani, Gunavardhan Kakulapati, Alex Pilchin, Swaminathan Sivasubramanian, Peter Vosshall, and Werner Vogels. Dynamo: amazon's highly available 
key-value store. In Proceedings of twenty-first ACM SIGOPS symposium on Operating systems principles, pages 205–220. ACM, 2007.

