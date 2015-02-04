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

	~TreeNode()
	{
		std::list<TreeNode<typename T>*>::iterator it =	m_children.begin(), it_end = m_children.end();

		for (; it != it_end; ++it) {
			if (*it) {
				delete *it;
				*it = 0;
			}
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

	// allow direct modification
	T	val;

private:
	TreeNode<T>*						m_parent;
	std::list<TreeNode<typename T>*>	m_children;
};

#endif