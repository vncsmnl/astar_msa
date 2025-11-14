"""
Sequence management service
Handles scanning and organizing sequence files
"""

from app.config import SOURCES
from app.utils.filesystem import scan_directory_recursively


def scan_all_sequences():
    """Scan all sequence sources and return organized structure"""
    all_sequences = {}

    for source_key, source_info in SOURCES.items():
        source_path = source_info['path']

        if not source_path.exists():
            continue

        source_structure = {
            'name': source_info['name'],
            'description': source_info['description'],
            'path': str(source_path),
            'categories': {}
        }

        # Scan the source directory
        categories = scan_directory_recursively(source_path)

        if categories:
            source_structure['categories'] = categories
            all_sequences[source_key] = source_structure

    return all_sequences
