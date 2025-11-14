"""
Filesystem utility functions
Handles directory scanning and file operations
"""

from pathlib import Path
from app.config import MAX_DIRECTORY_DEPTH, FASTA_EXTENSIONS


def scan_directory_recursively(directory, max_depth=MAX_DIRECTORY_DEPTH, current_depth=0):
    """Recursively scan directory for FASTA files and subdirectories"""
    structure = {}

    if not directory.exists() or current_depth >= max_depth:
        return structure

    # Get all items in directory
    try:
        items = sorted(directory.iterdir())
    except PermissionError:
        return structure

    # Separate directories and files
    subdirs = [item for item in items if item.is_dir()]
    fasta_files = [item.name for item in items if item.is_file() and item.suffix in FASTA_EXTENSIONS]

    # If there are FASTA files in this directory, add them
    if fasta_files:
        structure['_files'] = fasta_files

    # Recursively scan subdirectories
    for subdir in subdirs:
        subdir_structure = scan_directory_recursively(subdir, max_depth, current_depth + 1)
        if subdir_structure:  # Only add if not empty
            structure[subdir.name] = subdir_structure

    return structure
