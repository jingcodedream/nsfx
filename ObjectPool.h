/*
 * ObjectPool.h
 *
 *  Created on: 2015年11月18日
 *      Author: joe
 */

#ifndef OBJECTPOOL_H_
#define OBJECTPOOL_H_

#include <stddef.h>
#include <assert.h>

template<class T>
class ObjectPool
{
public:
    ObjectPool():m_free_list(NULL),m_num_each_block(100),m_cur_block(0),m_total_block(64)
    {
        m_node_block_array = (NodeBlock*)malloc(m_total_block*sizeof(NodeBlock));
        assert(m_node_block_array != NULL);
        m_node_block_array[m_cur_block].index = 0;
        m_node_block_array[m_cur_block].element_array = (T*)malloc(m_num_each_block*sizeof(T));
        assert(m_node_block_array[m_cur_block].element_array != NULL);
    }
    ~ObjectPool()
    {
        for(int i = 0; i < m_total_block; ++i)
        {
            if(m_node_block_array[i].element_array == NULL)
                break;
            free(m_node_block_array[i].element_array);
        }
        free(m_node_block_array);
    }

    T* get();
    void put(T* node);

private:
    int m_num_each_block;
    int m_cur_block;
    int m_total_block;
    T* m_free_list;

    struct NodeBlock
    {
        int index;
        void *element_array;
    };
    NodeBlock *m_node_block_array;
};

template<class T>
T* ObjectPool<T>::get()
{
    T *node = NULL;
    if(m_free_list != NULL)
    {
        node = m_free_list;
        m_free_list = m_free_list->next;
    }else
    {
        if(m_node_block_array[m_cur_block].index >= m_num_each_block)
        {
            m_cur_block += 1;
            if(m_cur_block > m_total_block)
            {
                m_total_block *= 2;
                m_node_block_array = (NodeBlock *)realloc(m_node_block_array, m_total_block);
                assert(m_node_block_array != NULL);
            }
            m_node_block_array[m_cur_block].index = 0;
            m_node_block_array[m_cur_block].element_array = (T*)malloc(m_num_each_block*sizeof(T));
            assert(m_node_block_array[m_cur_block].element_array != NULL);
        }
        node = m_node_block_array[m_cur_block].element_array[index];
        ++m_node_block_array[m_cur_block].index;
    }
    return node;
}

template<class T>
void ObjectPool<T>::put(T* node)
{
    assert(node != NULL);
    node->next = m_free_list;
    m_free_list = node;
}

#endif /* OBJECTPOOL_H_ */
