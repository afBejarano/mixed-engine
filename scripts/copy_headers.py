#!/usr/bin/env python3
"""
Copy header-only libraries to external directory after bootstrapping
"""

import os
import shutil
from pathlib import Path

def main():
    external_dir = Path('external')
    src_dir = external_dir / 'src'
    
    # Copy tiny_obj_loader.h
    tiny_obj_src = src_dir / 'tiny_obj_loader' / 'tiny_obj_loader.h'
    if tiny_obj_src.exists():
        shutil.copy2(tiny_obj_src, external_dir / 'tiny_obj_loader.h')
        print("Copied tiny_obj_loader.h")
    
    # Copy stb_image.h
    stb_src = src_dir / 'stb' / 'stb_image.h'
    if stb_src.exists():
        shutil.copy2(stb_src, external_dir / 'stb_image.h')
        print("Copied stb_image.h")
    
    # Copy rapidxml headers
    rapidxml_src = src_dir / 'rapidxml'
    if rapidxml_src.exists():
        for header in rapidxml_src.glob('*.hpp'):
            shutil.copy2(header, external_dir / header.name)
        print("Copied rapidxml headers")

if __name__ == '__main__':
    main() 