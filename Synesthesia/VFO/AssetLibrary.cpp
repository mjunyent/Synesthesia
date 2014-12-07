//
//  AssetLibrary.cpp
//  Synesthesia
//
//  Created by Marc Junyent Martín on 27/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#include "AssetLibrary.h"
#include <algorithm>

AssetLibrary::AssetLibrary(std::string url) : path(url) {
    try {
        if(!bfs::exists(path))
            (*Tobago.log)(Log::ERROR) << "Specified path doesn't exist: " << path.string();
        if(!bfs::is_directory(path))
            (*Tobago.log)(Log::ERROR) << "Specified path it's not a directory: " << path.string();

        std::vector<bfs::path> v;

        std::copy(bfs::directory_iterator(path), bfs::directory_iterator(),
                  std::back_inserter(v));
        
        std::sort(v.begin(), v.end());
        
        for(bfs::path& p : v) {
            cout << "    " << p << std::endl;
        }
    } catch (const bfs::filesystem_error& ex) {
        (*Tobago.log)(Log::ERROR) << "Exception trying to open " << path.string() << " : " << ex.what();
    }
}
