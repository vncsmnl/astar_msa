"""
Security utilities
Path validation and sanitization
"""

from pathlib import Path


def validate_path(file_path, allowed_base_paths):
    """
    Validate that a file path is within allowed directories
    Prevents directory traversal attacks
    """
    try:
        file_path = Path(file_path).resolve()
        
        # Check if path is within any allowed base path
        for base_path in allowed_base_paths:
            base_path = Path(base_path).resolve()
            try:
                file_path.relative_to(base_path)
                return True
            except ValueError:
                continue
        
        return False
    except (ValueError, OSError):
        return False
