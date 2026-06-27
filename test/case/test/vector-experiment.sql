-- =============================================
-- 任务一：向量类型数据的存储引擎支持
-- =============================================

-- 1.1 Schema 边界
create table t1_default(id int, emb vector);
create table t1_empty(emb vector());
create table t1_toobig(emb vector(16385));
create table t1_dim_ok(emb vector(4));
drop table t1_dim_ok;
drop table t1_default;
drop table t1_empty;

-- 1.2 向量数据插入与格式校验
create table t1_insert(id int, emb vector(3), tag char);
insert into t1_insert values(1, string_to_vector('[1.0, 0.0, 0.0]'), 'a');
insert into t1_insert values(2, string_to_vector('[3, 0, 0]'), 'b');
insert into t1_insert values(3, string_to_vector('[-1.5, 2.0, 3.0]'), 'c');
insert into t1_insert values(4, string_to_vector('[1, 2]'), 'bad_dim');
insert into t1_insert values(5, string_to_vector('[1, 2, 3, 4]'), 'bad_dim');
insert into t1_insert values(6, string_to_vector('[not-a-vector]'), 'bad_fmt');
select * from t1_insert;
select id, tag from t1_insert;
delete from t1_insert where id = 3;
select id, tag from t1_insert;
drop table t1_insert;

-- 1.3 向量转换函数
select vector_to_string(string_to_vector('[-1.5, 0, 2.25]')) as roundtrip;
create table t1_conv(id int, emb vector(3));
insert into t1_conv values(1, string_to_vector('[1, 2, 3]'));
insert into t1_conv values(2, string_to_vector('[4, 5, 6]'));
select id, vector_to_string(emb) as emb_str from t1_conv;
drop table t1_conv;

-- 1.4 向量比较运算
create table t1_cmp(id int, emb vector(3));
insert into t1_cmp values(1, string_to_vector('[1,0,0]'));
insert into t1_cmp values(2, string_to_vector('[3,0,0]'));
select id from t1_cmp where emb = string_to_vector('[3,0,0]');
select id from t1_cmp where emb <> string_to_vector('[3,0,0]');
drop table t1_cmp;

-- =============================================
-- 任务二：三种向量相似度距离计算
-- =============================================

-- 2.1 欧氏距离 EUCLIDEAN (及其别名 L2, L2_DISTANCE)
select distance(string_to_vector('[1,2]'), string_to_vector('[4,6]'), euclidean) as l2;
select distance(string_to_vector('[0,0,0]'), string_to_vector('[3,4,0]'), euclidean) as l2;
select distance(string_to_vector('[0,0,0]'), string_to_vector('[0,0,0]'), euclidean) as zero;
select distance(string_to_vector('[-1,-2]'), string_to_vector('[1,2]'), euclidean) as neg;
select distance(string_to_vector('[1,2]'), string_to_vector('[4,6]'), 'L2') as l2_alias;
select distance(string_to_vector('[1,2]'), string_to_vector('[4,6]'), 'L2_DISTANCE') as l2d_alias;

-- 2.2 余弦距离 COSINE (及其别名 COSINE_DISTANCE)
select distance(string_to_vector('[1,0]'), string_to_vector('[2,0]'), cosine) as cos_same;
select distance(string_to_vector('[1,0]'), string_to_vector('[0,1]'), cosine) as cos_orth;
select distance(string_to_vector('[1,0]'), string_to_vector('[-1,0]'), cosine) as cos_opp;
select distance(string_to_vector('[0,0]'), string_to_vector('[1,0]'), cosine) as cos_zero;
select distance(string_to_vector('[1,0]'), string_to_vector('[2,0]'), 'COSINE_DISTANCE') as cos_alias;

-- 2.3 内积距离 DOT / INNER_PRODUCT
select distance(string_to_vector('[1,2,3]'), string_to_vector('[4,5,6]'), dot) as dot;
select distance(string_to_vector('[1,2,3]'), string_to_vector('[4,5,6]'), 'INNER_PRODUCT') as dot_alias;
select distance(string_to_vector('[1,-2]'), string_to_vector('[-1,2]'), dot) as dot_neg;

-- 2.4 异常边界
select distance(string_to_vector('[1,0]'), string_to_vector('[0,0]'), unknown_method) as bad;
select distance(string_to_vector('[1,2]'), string_to_vector('[1,2,3]'), euclidean) as dim_mismatch;

-- =============================================
-- 任务三：向量距离精确查询与结果排序
-- =============================================

-- 3.1 数据准备
create table t3_points(id int, emb vector(3), tag char);
insert into t3_points values(1, string_to_vector('[1, 0, 0]'), 'a');
insert into t3_points values(2, string_to_vector('[3, 0, 0]'), 'b');
insert into t3_points values(3, string_to_vector('[6, 0, 0]'), 'c');
insert into t3_points values(4, string_to_vector('[-2, 0, 0]'), 'd');
insert into t3_points values(5, string_to_vector('[0, 5, 0]'), 'e');
insert into t3_points values(6, string_to_vector('[0, 0, 10]'), 'f');

-- 3.2 EUCLIDEAN Top-K
select id, distance(emb, string_to_vector('[0,0,0]'), euclidean) as dis from t3_points order by dis asc limit 2;
select id, distance(emb, string_to_vector('[0,0,0]'), euclidean) as dis from t3_points order by dis desc limit 2;

-- 3.3 COSINE Top-K
select id, distance(emb, string_to_vector('[1,0,0]'), cosine) as dis from t3_points order by dis asc limit 3;

-- 3.4 DOT Top-K
select id, distance(emb, string_to_vector('[1,0,0]'), 'DOT') as score from t3_points order by score desc limit 2;

-- 3.5 别名与投影
select id, tag, distance(emb, string_to_vector('[0,5,0]'), euclidean) as dist_to_e from t3_points order by dist_to_e asc limit 4;
select id, distance(emb, string_to_vector('[0,0,0]'), euclidean) as dis from t3_points order by dis asc;

drop table t3_points;

-- =============================================
-- 任务四：IVF_Flat 向量索引构建与近似检索
-- =============================================

-- 4.1 索引语法与默认参数
create table t4_base(id int, emb vector(3), tag char);
insert into t4_base values(1, string_to_vector('[1, 0, 0]'), 'a');
insert into t4_base values(2, string_to_vector('[3, 0, 0]'), 'b');
insert into t4_base values(3, string_to_vector('[6, 0, 0]'), 'c');
insert into t4_base values(4, string_to_vector('[-2, 0, 0]'), 'd');
create vector index idx4_default on t4_base(emb);
select id, distance(emb, string_to_vector('[0,0,0]'), euclidean) as dis from t4_base order by dis asc limit 2;
create vector index idx4_bad on t4_base(tag);
create vector index idx4_bad_lists on t4_base(emb) with (lists=0);
drop table t4_base;

-- 4.2 索引自定义参数
create table t4_custom(id int, emb vector(2));
insert into t4_custom values(1, string_to_vector('[1.5, -2.0]'));
insert into t4_custom values(2, string_to_vector('[0, 3.0]'));
insert into t4_custom values(3, string_to_vector('[-1, 1.5]'));
insert into t4_custom values(4, string_to_vector('[2.5, 0.5]'));
create vector index idx4_custom on t4_custom(emb) with (distance=cosine, type=ivfflat, lists=2, probes=1);
select id from t4_custom order by distance(emb, string_to_vector('[1,0]'), cosine) asc limit 1;
drop table t4_custom;

-- 4.3 索引命中与聚类验证
create table t4_cluster(id int, emb vector(2), label char);
insert into t4_cluster values(1, string_to_vector('[-5.1, -5.2]'), 'A');
insert into t4_cluster values(2, string_to_vector('[-4.9, -4.8]'), 'A');
insert into t4_cluster values(3, string_to_vector('[-5.2, -5.0]'), 'A');
insert into t4_cluster values(4, string_to_vector('[-4.8, -5.3]'), 'A');
insert into t4_cluster values(5, string_to_vector('[-5.0, -4.9]'), 'A');
insert into t4_cluster values(6, string_to_vector('[5.1, 5.2]'), 'B');
insert into t4_cluster values(7, string_to_vector('[4.9, 4.8]'), 'B');
insert into t4_cluster values(8, string_to_vector('[5.2, 5.0]'), 'B');
insert into t4_cluster values(9, string_to_vector('[4.8, 5.3]'), 'B');
insert into t4_cluster values(10, string_to_vector('[5.0, 4.9]'), 'B');
insert into t4_cluster values(11, string_to_vector('[0, 0]'), 'O');
create vector index idx4_cluster on t4_cluster(emb) with (distance=euclidean, type=ivfflat, lists=2, probes=1);
select id, label, distance(emb, string_to_vector('[-5, -5]'), euclidean) as dis from t4_cluster order by dis asc limit 3;
drop table t4_cluster;

-- 4.4 索引维护与 DDL
create table t4_rebuild(id int, emb vector(3));
insert into t4_rebuild values(1, string_to_vector('[1,2,3]'));
create vector index idx4_rebuild on t4_rebuild(emb);
select id, distance(emb, string_to_vector('[1,2,3]'), euclidean) as dis from t4_rebuild order by dis asc limit 1;
drop table t4_rebuild;
