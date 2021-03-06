// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_LEDGER_SRC_CLOUD_SYNC_PUBLIC_USER_CONFIG_H_
#define APPS_LEDGER_SRC_CLOUD_SYNC_PUBLIC_USER_CONFIG_H_

#include <string>

namespace cloud_sync {

// Sync configuration for a particular user.
struct UserConfig {
  bool use_sync = false;
  // ID of the firebase instance.
  std::string server_id;
  // The id of the user.
  std::string user_id;
};

}  // namespace cloud_sync

#endif  // APPS_LEDGER_SRC_CLOUD_SYNC_PUBLIC_USER_CONFIG_H_
