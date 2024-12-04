from RLTest import Env
from common import *
import os

# @skip(True)
def test01():
    env = Env(noDefaultModuleArgs=True, module='', moduleArgs='')
    redisearch_module_path = os.getenv('MODULE')

    env.start()
    res = env.cmd('MODULE', 'LIST')
    env.assertEqual(res, [])
    env.expect('MODULE', 'LOADEX', redisearch_module_path,
            'CONFIG', 'invalid_config', 1).error()\
            .contains('Error loading the extension')
    env.assertTrue(env.isUp())
    env.stop()

def test02():
    var_name = 'MODULE'
    var_value =  os.getenv(var_name)
    print("var_name: ", var_name)
    print(var_name, var_value)


def testModuleLoadexInvalidConfig():
    env = Env(noDefaultModuleArgs=True)
    skipOnExistingEnv(env)

    dbFileName = env.cmd('config', 'get', 'dbfilename')[1]
    dbDir = env.cmd('config', 'get', 'dir')[1]
    rdbFilePath = os.path.join(dbDir, dbFileName)
    env.stop()
    os.unlink(rdbFilePath)

    # Remove modules and args
    env.assertEqual(len(env.envRunner.modulePath), 2)
    env.assertEqual(len(env.envRunner.moduleArgs), 2)
    redisearch_module_path = env.envRunner.modulePath[0]
    print(redisearch_module_path)
    env.envRunner.modulePath.pop()
    env.envRunner.moduleArgs.pop()
    env.envRunner.modulePath.pop()
    env.envRunner.moduleArgs.pop()
    env.envRunner.masterCmdArgs = env.envRunner.createCmdArgs('master')
    print(env.envRunner.masterCmdArgs)

    env.start()
    res = env.cmd('MODULE', 'LIST')
    env.assertEqual(res, [])
    env.expect('MODULE', 'LOADEX', redisearch_module_path, 'CONFIG',
               'invalid_config', 1).error()\
                .contains('Error loading the extension')
    env.assertTrue(env.isUp())
    env.stop()
