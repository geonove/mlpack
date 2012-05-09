/**
 * @file dual_tree_depth_first_traverser.hpp
 * @author Ryan Curtin
 *
 * A meta-heuristic-type class (is that even a word?) which allows an automatic
 * way to perform a dual-tree traversal of two trees.  The trees must be the
 * same type.
 */
#ifndef __MLPACK_CORE_TREE_TRAVERSERS_DUAL_TREE_DEPTH_FIRST_TRAVERSER_HPP
#define __MLPACK_CORE_TREE_TRAVERSERS_DUAL_TREE_DEPTH_FIRST_TRAVERSER_HPP

#include <mlpack/core.hpp>
#include <stack>

namespace mlpack {
namespace tree {

template<typename TreeType, typename RuleType>
class DualTreeDepthFirstTraverser
{
 public:
  DualTreeDepthFirstTraverser(RuleType& rule) :
      rule(rule), numPrunes(0)
  { /* Nothing to do. */ }

  void Traverse(TreeType& queryNode, TreeType& referenceNode)
  {
    // Each of these stacks should have the same number of elements in them.
    std::stack<TreeType*> queryStack;
    std::stack<TreeType*> referenceStack;
    queryStack.push(&queryNode);
    referenceStack.push(&referenceNode);

    while (!queryStack.empty())
    {
      TreeType& query = *queryStack.top();
      TreeType& reference = *referenceStack.top();
      queryStack.pop();
      referenceStack.pop();

      // Check if pruning can occur.
      if (rule.CanPrune(query, reference))
      {
        numPrunes++;
        return;
      }

      // Run the base case for any points.  We must have points in both trees.
      if ((query.NumPoints() > 0) && (reference.NumPoints() > 0))
      {
        for (size_t i = 0; i < query.NumPoints(); ++i)
          for (size_t j = 0; j < reference.NumPoints(); ++j)
            rule.BaseCase(query.Point(i), reference.Point(j));
      }

      // Find the correct way to recurse given these two points.
      arma::Mat<size_t> recursionOrder;
      bool queryRecurse;
      bool referenceRecurse;
      rule.RecursionOrder(query, reference, recursionOrder, queryRecurse,
          referenceRecurse);

      // The RuleType has told us what we need to recurse on; act accordingly.
      if (queryRecurse && referenceRecurse) // Recurse on both trees.
      {
        for (size_t i = 0; i < recursionOrder.n_cols; ++i)
        {
          queryStack.push(&(query.Child(recursionOrder(0, i))));
          referenceStack.push(&(reference.Child(recursionOrder(1, i))));
        }
      }
      else if (queryRecurse) // Only recurse on query tree.
      {
        for (size_t i = 0; i < recursionOrder.n_cols; ++i)
        {
          queryStack.push(&(query.Child(recursionOrder(0, i))));
          referenceStack.push(&reference);
        }
      }
      else if (referenceRecurse) // Only recurse on reference tree.
      {
        for (size_t i = 0; i < recursionOrder.n_cols; ++i)
        {
          queryStack.push(&query);
          referenceStack.push(&(reference.Child(recursionOrder(1, i))));
        }
      }

      // Now, after recursion, update any information.
      rule.UpdateAfterRecursion(query, reference);
    }
  }

  size_t NumPrunes() const { return numPrunes; }
  size_t& NumPrunes() { return numPrunes; }

 private:
  RuleType& rule;
  size_t numPrunes;
};

}; // namespace tree
}; // namespace mlpack

#endif // __MLPACK_CORE_TREE_TRAVERSERS_DUAL_TREE_DEPTH_FIRST_TRAVERSER_HPP

