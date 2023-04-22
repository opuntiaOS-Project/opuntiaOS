/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Mainasara Tsowa <tsowamainasara@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Begining of possible AssetManager to locate/read/write to files and folders in the app directory
namespace LFoundation {

class AssetManager {
public:
    AssetManager(const std::string& name)
        :m_app_root(std::string() + "/Applications/" + name + ".app/Content") 
    {
    }

    ~AssetManager() = default;

    std::string find(std::string path)
    {
        return m_app_root + path;
    }
private:
    std::string m_app_root;
};

}