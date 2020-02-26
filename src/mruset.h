#include "trace-log.h" //++++++++++++++++++
// Copyright (c) 2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MRUSET_H
#define BITCOIN_MRUSET_H

#include <deque>
#include <set>
#include <utility>

/** STL-like set container that only keeps the most recent N elements. */
template <typename T>
class mruset
{
public:
    typedef T key_type;
    typedef T value_type;
    typedef typename std::set<T>::iterator iterator;
    typedef typename std::set<T>::const_iterator const_iterator;
    typedef typename std::set<T>::size_type size_type;

protected:
    std::set<T> set;
    std::deque<T> queue;
    size_type nMaxSize;

public:
    mruset(size_type nMaxSizeIn = 0) { nMaxSize = nMaxSizeIn; }
    iterator begin() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return set.begin(); }
    iterator end() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return set.end(); }
    size_type size() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return set.size(); }
    bool empty() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return set.empty(); }
    iterator find(const key_type& k) const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return set.find(k); }
    size_type count(const key_type& k) const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return set.count(k); }
    void clear()
    {

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
        set.clear();
        queue.clear();
    }
    bool inline friend operator==(const mruset<T>& a, const mruset<T>& b) { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return a.set == b.set; }
    bool inline friend operator==(const mruset<T>& a, const std::set<T>& b) { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return a.set == b; }
    bool inline friend operator<(const mruset<T>& a, const mruset<T>& b) { return a.set < b.set; }
    std::pair<iterator, bool> insert(const key_type& x)
    {

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
        std::pair<iterator, bool> ret = set.insert(x);
        if (ret.second) {
            if (nMaxSize && queue.size() == nMaxSize) {
                set.erase(queue.front());
                queue.pop_front();
            }
            queue.push_back(x);
        }
        return ret;
    }
    size_type max_size() const { 
	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
return nMaxSize; }
    size_type max_size(size_type s)
    {

	FUNC_LOG_TRACE();//+++++++++++++++++++++++++++
        if (s)
            while (queue.size() > s) {
                set.erase(queue.front());
                queue.pop_front();
            }
        nMaxSize = s;
        return nMaxSize;
    }
};

#endif // BITCOIN_MRUSET_H
