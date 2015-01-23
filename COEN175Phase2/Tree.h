#ifndef __TREE_H__
#define __TREE_H__

#include <list>

template <typename T>
class TreeNode
{
public:
	TreeNode(TreeNode<T>* parent = 0)
		: m_parent(parent)
	{
		parent->addChild(this);
	}

	void setParent(TreeNode<T>* parent)
	{
		m_parent = parent;
		parent->addChild(this);
	}

	typename std::list<TreeNode<T>*>::const_iterator getChildIterator() const
	{
		return m_children.begin();
	}

	typename std::list<TreeNode<T>*>::iterator		getChildIterator()
	{
		return m_children.begin();
	}

	const TreeNode<T>&	getParent() const
	{
		return *m_parent;
	}

	TreeNode<T>&	getParent()
	{
		return *m_parent;
	}

	T& getVal() { return m_values; }
	const T& getVal() const { return m_values; }

private:
	void addChild(TreeNode<T>* child)
	{
		m_children.push_back(child);
	}

	TreeNode<T>*			m_parent;
	std::list<TreeNode<typename T>*> m_children;
	T						m_values;
};

#endif