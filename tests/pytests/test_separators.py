# -*- coding: utf-8 -*-

# from common import (assertInfoField, waitForIndex)
from common import *

def test_Separators(env):
    # Index with custom separators
    env.expect('FT.CREATE', 'idx1', 'ON', 'HASH', 'SEPARATORS', ' \t!\"#$%&\'()*+,-./:;<=>?@[\]^`{|}~', 'schema', 'foo', 'text').ok()
    assertInfoField(env, 'idx1', 'separators', [' \t!\"#$%&\'()*+,-./:;<=>?@[\]^`{|}~'])
    env.cmd('FT.DROPINDEX', 'idx1')

def test_IndexOnHashWithCustomSeparators(env):
    # Create sample data
    env.cmd('HSET customer:1  code 101;111 email c01@rx.com name Kyle')
    env.cmd('HSET customer:2  code 101;222 email c02@rx.com name Sarah')
    env.cmd('HSET customer:3  code 101;333 email c03@rx.com name Ginger')

    # Index with default separators
    env.expect('FT.CREATE idx1 ON hash PREFIX 1 customer: SCHEMA code TEXT SORTABLE email TEXT SORTABLE name TEXT SORTABLE').equal('OK')
    waitForIndex(env, 'idx1')

    # res = env.cmd('FT.SEARCH', 'idx1:customer', '@email:c01\\@rx\\.com', 'RETURN', '1', 'email')
    # env.assertEqual(res, [0])

    res = env.execute_command('HMGET customer:2 name email code')
    print(res)

    res = env.execute_command('FT.SEARCH idx1 @name:Sarah')
    print(res)

    res = env.execute_command('FT.SEARCH idx1 @code:101 RETURN 1 code')
    print(res)

    env.cmd('FT.DROPINDEX', 'idx1')

    # Index with custom separators
    env.expect('FT.CREATE', 'idx2', 'SEPARATORS', ' \t!\"#$%&\'()*+,-./:<=>?@[\]^`{|}~', 'ON', 'hash', 'PREFIX', '1', '"customer:"', 'SCHEMA', 'code', 'TEXT', 'SORTABLE', 'email', 'TEXT', 'SORTABLE', 'name', 'TEXT', 'SORTABLE').equal('OK')
    waitForIndex(env, 'idx2')
    env.expect('FT.INFO idx2')
    assertInfoField(env, 'idx2', 'separators', [' \t!\"#$%&\'()*+,-./:<=>?@[\]^`{|}~'])

    res = env.execute_command('FT.SEARCH idx2 @code:101 RETURN 1 code')
    print(res)

    res = env.execute_command('FT.SEARCH idx2 @code:101\\;111 RETURN 1 code')
    print(res)

    # # res = env.cmd('FT.SEARCH', 'idx2', '@email:c01\\@rx.com', 'RETURN', '1', 'email')
    # # print(res)
    # # env.assertEqual(res, [1, 'customer:1', ['email', 'c01@rx.com']])
    # # res = env.cmd('FT.SEARCH', 'idx2', '@code:101~222', 'RETURN', '1', 'email')
    # # print(res)
    # res = env.cmd('HGET customer:1 email ')
    # print(res)
    # res = env.cmd('FT.SEARCH', 'idx2', '@name:Sarah')
    # print(res)
    # env.cmd('FT.DROP', 'idx2')
    
