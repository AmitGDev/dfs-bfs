/*
    main.cpp
    Copyright (c) 2024-2026, Amit Gefen

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <memory>
#include <print>
#include <queue>
#include <stack>
#include <vector>

namespace  // (Anonymous namespace)
{

// **Preorder and Postorder traversals primarily belong to Depth-First Search
// (DFS).** They define the order in which nodes are visited during a DFS
// exploration. (However, they are not limited to DFS and can be applied to
// other tree traversals.)
//
// **Preorder:** Visits the root node first, followed by its left child, then
// its right child.
// Subtrees are then traversed recursively in the same order.
// **Postorder:** Visits the left child first, then the right child, and finally
// the root node.
// Subtrees are then traversed recursively in the same order.
//
// **Breadth-First Search (BFS)** employs a distinct level-order traversal
// approach. It visits all nodes at a specific level before moving to the next
// level. This order is particularly useful for finding the shortest path in an
// unweighted graph.
//
// **Remember:** Preorder and Postorder are not directly applicable to BFS.
// When dealing with BFS, focus on visiting nodes level by level, not their
// individual order within those levels.

enum class DfsTraversal : uint8_t { kPreOrder, kPostOrder };

// A node owns its children, so the tree has exactly one owner and is freed
// when the root goes out of scope. It is an aggregate on purpose: that lets
// std::make_unique build whole subtrees in a single expression.
struct Node {
  int value{0};
  std::unique_ptr<Node> left{nullptr};
  std::unique_ptr<Node> right{nullptr};
};

}  // namespace

// Traversals take a non-owning `const Node*` because they only observe the
// tree; ownership stays with the caller. A null pointer is an empty tree.

/*
         1
        / \
       2   4
      /     \
     3       5
*/
[[nodiscard]] static std::unique_ptr<Node> BuildTree() {
  return std::make_unique<Node>(
      1, std::make_unique<Node>(2, std::make_unique<Node>(3)),
      std::make_unique<Node>(4, nullptr, std::make_unique<Node>(5)));
}

// Recursion needs an accumulator shared across calls; keeping it in this
// helper lets the public DfsR return its result by value.
// Recursion is the point of this variant (it is the counterpart of the
// stack-based Dfs below), so misc-no-recursion is suppressed intentionally.
// NOLINTNEXTLINE(misc-no-recursion)
static void DfsRImpl(const Node* node, const DfsTraversal traversal,
                     std::vector<int>& output) {
  if (node == nullptr) {  // Empty tree.
    return;
  }

  if (traversal == DfsTraversal::kPreOrder) {
    output.push_back(node->value);
  }

  DfsRImpl(node->left.get(), traversal, output);
  DfsRImpl(node->right.get(), traversal, output);

  if (traversal == DfsTraversal::kPostOrder) {
    output.push_back(node->value);
  }
}

// Depth-First Search (DFS) traversal, recursive.
// Time complexity: O(n)
[[nodiscard]] static std::vector<int>
DfsR(const Node* root, const DfsTraversal traversal = DfsTraversal::kPreOrder) {
  std::vector<int> output;
  DfsRImpl(root, traversal, output);
  return output;
}

// Depth-First Search (DFS) traversal, using stack.
// Time complexity: O(n)
[[nodiscard]] static std::vector<int>
Dfs(const Node* root, const DfsTraversal traversal = DfsTraversal::kPreOrder) {
  std::vector<int> output;
  if (root == nullptr) {  // Empty tree.
    return output;
  }

  const bool is_preorder = traversal == DfsTraversal::kPreOrder;

  std::stack<const Node*> stack;  // ** Using stack. **
  stack.push(root);

  while (!stack.empty()) {
    const Node* const current = stack.top();
    stack.pop();

    output.push_back(current->value);

    // The stack pops the child pushed last, so push order is the reverse of
    // the visit order we want. Preorder visits root-left-right: push right
    // first. Postorder is built as root-right-left (push left first) and
    // reversed below, which yields left-right-root without a second stack.
    const Node* const first_pushed =
        is_preorder ? current->right.get() : current->left.get();
    const Node* const second_pushed =
        is_preorder ? current->left.get() : current->right.get();

    if (first_pushed != nullptr) {
      stack.push(first_pushed);
    }

    if (second_pushed != nullptr) {
      stack.push(second_pushed);
    }
  }

  if (!is_preorder) {
    std::ranges::reverse(output);
  }

  return output;
}

// Breadth-First Search (BFS) traversal
// Time complexity: O(n), assume O(1) add/remove queue operation.
// Space: O(n) at most, probably less.
[[nodiscard]] static std::vector<int> Bfs(const Node* root) {
  std::vector<int> output;
  if (root == nullptr) {  // Empty tree.
    return output;
  }

  std::queue<const Node*> queue;  // ** Using queue. **
  queue.push(root);

  while (!queue.empty()) {
    const Node* const current = queue.front();
    queue.pop();

    output.push_back(current->value);

    if (current->left != nullptr) {
      queue.push(current->left.get());
    }

    if (current->right != nullptr) {
      queue.push(current->right.get());
    }
  }

  return output;
}

int main() {
  // std::println can throw (e.g. std::bad_cast from locale facets); catching
  // here keeps main noexcept-in-practice (bugprone-exception-escape).
  try {
    const auto root = BuildTree();

    std::println("DfsR (preorder):\t {}",
                 DfsR(root.get(), DfsTraversal::kPreOrder));  // Recursive
    std::println("DfsR (postorder):\t {}",
                 DfsR(root.get(), DfsTraversal::kPostOrder));

    std::println("Dfs (preorder):\t\t {}",
                 Dfs(root.get(), DfsTraversal::kPreOrder));  // Stack
    std::println("Dfs (postorder):\t {}",
                 Dfs(root.get(), DfsTraversal::kPostOrder));

    std::println("Bfs:\t\t\t {}", Bfs(root.get()));  // Queue
    return EXIT_SUCCESS;
  } catch (const std::exception& e) {
    // fputs instead of std::println: the error path must not throw again.
    std::fputs(e.what(), stderr);
    std::fputs("\n", stderr);
    return EXIT_FAILURE;
  }
}
