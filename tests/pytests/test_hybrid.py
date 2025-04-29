# -*- coding: utf-8 -*-
from RLTest import Env
from common import *
from includes import *
from random import randrange
import base64

def test_hybrid_query_combined_scores():
    env = Env(moduleArgs='DEFAULT_DIALECT 2')
    conn = getConnectionByEnv(env)

    # Define index parameters
    dim = 4
    n_vectors = 10
    k = 5  # number of nearest neighbors to return

    # Create index with both vector and text fields
    env.expect('FT.CREATE', 'idx', 'SCHEMA',
              'v', 'VECTOR', 'FLAT', '6', 'TYPE', 'FLOAT32', 'DIM', dim, 'DISTANCE_METRIC', 'L2',
              't', 'TEXT').ok()

    # Insert documents with controlled vector values and text
    p = conn.pipeline(transaction=False)
    for i in range(n_vectors):
        # Create vectors where values increase with i
        vector = create_np_array_typed([i*1] * dim, 'FLOAT32')

        # Alternate between 'common' and 'rare' text
        text = 'common text' if i % 2 == 0 else 'rare text'

        p.execute_command('HSET', i,
                         'v', vector.tobytes(),
                         't', text)
        print(f"echo -ne {vector.tobytes()} | redis-cli HSET {i} t \"{text}\" v")

    p.execute()

    # Create query vector - should be closest to middle vectors
    query_vec = create_np_array_typed([4] * dim, 'FLOAT32')

    # Test cases with different hybrid queries
    test_cases = [
        {
            'query': f'(@t:common)=>[KNN {k} @v $vec_param AS score]',
            'expected_ids': ['4', '2', '6', '0', '8'],  # Ordered by vector similarity
            'desc': "Common text + vector similarity"
        },
        {
            'query': f'(@t:rare)=>[KNN {k} @v $vec_param AS score]',
            'expected_ids': ['3', '5', '1', '7', '9'],
            'desc': "Rare text + vector similarity"
        },
        {
            'query': f'(@t:(common|rare))=>[KNN {k} @v $vec_param AS score]',
            'expected_ids': ['4', '3', '5', '2', '6'],
            'desc': "All texts + vector similarity"
        },
        {
            'query': f'(@t:(common|rare))=>[KNN {k} @v $vec_param AS score]',
            'expected_ids': ['4', '3', '5', '2', '6'],
            'desc': "All texts + vector similarity"
        }
    ]

    print('query_vec:', query_vec.tobytes() )

    for case in test_cases:
        # Execute hybrid query
        res = env.cmd('FT.SEARCH', 'idx', case['query'],
                     'PARAMS', 2, 'vec_param', query_vec.tobytes(),
                     'RETURN', 2, 'score', 't', 'SORTBY', 'score')

        # First element is the number of results
        n_results = res[0]

        # Extract document IDs from results
        result_ids = [res[2*i+1] for i in range(min(k, n_results))]

        # Verify results
        env.assertEqual(result_ids, case['expected_ids'],
                       message=f"Failed case: {case['desc']}\n" +
                       f"Expected: {case['expected_ids']}\n" +
                       f"Got: {result_ids}")

        # Verify each result has both vector score and text field
        for i in range(min(k, n_results)):
            result_dict = {res[2*i+2][j]: res[2*i+2][j+1]
                         for j in range(0, len(res[2*i+2]), 2)}

            # Verify result contains both vector score and text field
            env.assertContains('score', result_dict)
            env.assertContains('t', result_dict)

