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
		if (parent) {
			parent->addChild(this);
		}
	}

	void setParent(TreeNode<T>* parent)
	{
		m_parent = parent;
		if (parent) {
			parent->addChild(this);
		}
	}

	typename const std::list<TreeNode<T>*>& getChildList() const
	{
		return m_children;
	}

	void addChild(TreeNode<T>* child)
	{
		if (child) {
			m_children.push_back(child);
			child->m_parent = this;
		}
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
	TreeNode<T>*			m_parent;
	std::list<TreeNode<typename T>*> m_children;
	T						m_values;
};

#endif