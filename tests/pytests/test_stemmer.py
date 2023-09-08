# -*- coding: utf-8 -*-

from includes import *
from common import *

def test01(env):
    env.expect('FT.CREATE', 'idx', 'ON', 'JSON',
               'SCHEMA',
               '$.id', 'AS', 'id', 'TAG', '$.brand', 'AS', 'brand', 'TEXT',
               '$.title', 'AS', 'title', 'TEXT',
               '$.category', 'AS', 'category', 'TEXT',
               '$.param', 'AS', 'param', 'TEXT',
               '$.tags', 'AS', 'tags', 'TEXT',
               '$.price', 'AS', 'price', 'NUMERIC').ok()
    
    # FT.CREATE idx ON JSON SCHEMA $.id AS id TAG $.brand AS brand TEXT $.title AS title TEXT $.category AS category TEXT $.param AS param TEXT $.tags AS tags TEXT $.price AS price NUMERIC
    
    # res = env.execute_command(

    # FT.EXPLAINCLI idx "@category:耳机 AND @brand:(华为) AND @price:[1500 2000]" LANGUAGE chinese
    # FT.EXPLAINCLI idx "@category:耳机 @brand:(华为) @price:[1500 2000]" LANGUAGE chinese

    # res = 

    # res = FT.EXPLAIN sku:idx "@category:耳机 AND @brand:(华为) AND @price:[1500 2000]" LANGUAGE chinese


#See bug: https://github.com/RediSearch/RediSearch/issues/3840

# FT.CREATE idx_hash ON HASH SCHEMA category TEXT

#  127.0.0.1:6379> FT.EXPLAINCLI idx_hash "@category:(蓝牙/无线耳机)" LANGUAGE chinese
#  1) @category:INTERSECT {
#  2)   @category:UNION {
#  3)     @category:蓝牙
#  4)     @category:蓝牙(expanded)
#  5)   }
#  6)   @category:UNION {
#  7)     @category:无线耳机
#  8)     @category:无线(expanded)
#  9)     @category:耳机(expanded)
# 10)   }
# 11) }

# 127.0.0.1:6379> FT.EXPLAINCLI idx_hash "@category:(Bluetooth/Wireless Headphones)" LANGUAGE english
#  1) @category:INTERSECT {
#  2)   @category:UNION {
#  3)     @category:bluetooth
#  4)     @category:+bluetooth(expanded)
#  5)   }
#  6)   @category:UNION {
#  7)     @category:wireless
#  8)     @category:+wireless(expanded)
#  9)   }
# 10)   @category:UNION {
# 11)     @category:headphones
# 12)     @category:+headphon(expanded)
# 13)     @category:headphon(expanded)
# 14)   }
# 15) }