// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/ledger/storage/impl/btree/btree_iterator.h"

#include "apps/ledger/storage/impl/store/tree_node.h"
#include "lib/ftl/logging.h"

namespace storage {

BTreeIterator::BTreeIterator(std::unique_ptr<const TreeNode> root) {
  // Initialize the iterator.
  std::unique_ptr<const TreeNode> current_node = std::move(root);
  while (current_node) {
    std::unique_ptr<const TreeNode> next_node;
    Status status = current_node->GetChild(0, &next_node);
    // We position the current reading pointer just "before" the beginning of
    // the node.
    stack_.emplace(std::move(current_node), -1, 0);
    if (status == Status::OK) {
      current_node = std::move(next_node);
    } else {
      break;
    }
  }
  this->Next();
}

BTreeIterator::~BTreeIterator() {}

BTreeIterator& BTreeIterator::Next() {
  FTL_DCHECK(!Done());
  // direction_up tells the algorithm whether we are exploring the tree down
  // (from root to leaves) or up (from leaves to root). In the down direction,
  // we are exploring the children of each node: when exploring a node, we look
  // for its next unexplored child and explore it, recursively, until we reach
  // the bottom of the tree. In the up direction, we are exploring the entries
  // of each node: starting at the bottom, we look at the next entry of the
  // explored node; if there is no such entry (we are at the end), we go to the
  // next node up the stack and look at its next entry, and again, until we
  // either find a valid entry or we unrolled the whole tree.
  // The normal mode of operation when nodes are not empty is to explore "down"
  // [3] until no child is found [4], then go "up" and return the next entry of
  // the bottom node [1]. If we are also at the end of the entry list, we go up
  // further [2].
  bool direction_up = false;
  while (!stack_.empty()) {
    Position* current_position = &stack_.top();
    if (direction_up) {
      // We are moving up in the tree, as we finished exploring the previous
      // branch. We now point to the next entry (key/value pair).
      current_position->entry_index++;
      if (current_position->entry_index <
          current_position->node->GetKeyCount()) {
        // [1] There is a next entry in this node, point to it and exit.
        current_position->node->GetEntry(current_position->entry_index,
                                         &current_entry_);
        break;
      }
      // [2] We are at the end of this node, so let's continue to move up the
      // tree.
      stack_.pop();
      continue;
    }

    // We are either at the beginning of a node, or already explored its entry
    // at entry_index, so we explore the next child of the node.
    current_position->child_index++;

    if (current_position->child_index <
        current_position->node->GetKeyCount() + 1) {
      // We are not at the end of the child list, given the number of
      // entries stored in this node. However, this doesn't mean that this
      // child won't be empty.
      std::unique_ptr<const TreeNode> next_child;
      Status status = current_position->node->GetChild(
          current_position->child_index, &next_child);

      if (status == Status::OK) {
        // [3] The child is not empty, so we add it to the stack to explore it.
        stack_.emplace(std::move(next_child), -1, -1);
      } else {
        // [4] The child is empty, so we reverse direction. This will lead us to
        // try to read the next entry, if available.
        direction_up = true;
      }
      continue;
    }
    // [5] We are at the end of the child list, so this node is completely
    // explored now. Remove it from the stack and go up.
    direction_up = true;
    stack_.pop();
  }
  return *this;
}

bool BTreeIterator::Done() {
  return stack_.empty();
}

const Entry& BTreeIterator::operator*() const {
  return current_entry_;
}

const Entry* BTreeIterator::operator->() const {
  return &current_entry_;
}

}  // namespace storage