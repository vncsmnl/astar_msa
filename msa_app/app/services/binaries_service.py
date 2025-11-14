"""
Binary management service
Handles scanning and validation of executable binaries
"""

import os
import subprocess
from pathlib import Path
from app.config import BIN_DIR


def check_binary_supports_threads(binary_path):
    """Check if a binary supports the -t/--threads option"""
    try:
        result = subprocess.run(
            [str(binary_path), '--help'],
            capture_output=True,
            text=True,
            timeout=5
        )
        help_text = result.stdout + result.stderr
        # Check if help text mentions threads option
        return '-t' in help_text or '--threads' in help_text
    except:
        return False


def get_available_binaries():
    """Scan bin directory and return available executables"""
    binaries = {
        'astar': [],
        'pastar': []
    }

    if not BIN_DIR.exists():
        return binaries

    # Scan for executables
    for binary_file in BIN_DIR.iterdir():
        if binary_file.is_file() and os.access(binary_file, os.X_OK):
            name = binary_file.name
            
            # Check if binary supports threads
            supports_threads = check_binary_supports_threads(binary_file)

            # Categorize by algorithm type
            if 'astar' in name.lower() and 'pastar' not in name.lower():
                binaries['astar'].append({
                    'name': name,
                    'path': str(binary_file),
                    'display_name': name.replace('msa_astar_', 'A-Star - ').replace('_', ' ').title(),
                    'supports_threads': supports_threads
                })
            elif 'pastar' in name.lower():
                binaries['pastar'].append({
                    'name': name,
                    'path': str(binary_file),
                    'display_name': name.replace('msa_pastar_', 'PA-Star - ').replace('_', ' ').title(),
                    'supports_threads': supports_threads
                })

    # Sort by name
    binaries['astar'].sort(key=lambda x: x['name'])
    binaries['pastar'].sort(key=lambda x: x['name'])

    return binaries


def get_binary_path(binary_name=None, algorithm=None):
    """
    Get the path and capabilities of a specific binary
    Returns (binary_path, supports_threads)
    """
    available_binaries = get_available_binaries()

    if binary_name:
        # User specified a specific binary
        binary_path = BIN_DIR / binary_name
        if not binary_path.exists() or not os.access(binary_path, os.X_OK):
            raise ValueError(f'Binary not found or not executable: {binary_name}')
        supports_threads = check_binary_supports_threads(binary_path)
        return binary_path, supports_threads
    elif algorithm:
        # Use default based on algorithm type
        if algorithm == 'msa_astar' and available_binaries['astar']:
            binary_info = available_binaries['astar'][0]
            return Path(binary_info['path']), binary_info['supports_threads']
        elif algorithm == 'msa_pastar' and available_binaries['pastar']:
            binary_info = available_binaries['pastar'][0]
            return Path(binary_info['path']), binary_info['supports_threads']
        else:
            raise ValueError('No binary available for selected algorithm')
    else:
        raise ValueError('Either binary_name or algorithm must be provided')
