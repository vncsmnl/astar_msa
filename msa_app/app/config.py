"""
Configuration module for MSA Flask application
Contains all paths and constants used across the application
"""

from pathlib import Path

# Base directory
BASE_DIR = Path(__file__).parent.parent.parent.absolute()
BIN_DIR = BASE_DIR / 'bin'
SEQS_DIR = BASE_DIR / 'seqs'
RESULTS_DIR = Path(__file__).parent.parent / 'results'

# Flask configuration
SECRET_KEY = 'msa-astar-pastar-secret-key'

# Define the 4 main sources
SOURCES = {
    'BALIBASE': {
        'name': 'BALIBASE',
        'path': SEQS_DIR / 'Balibase',
        'description': 'BALIBASE benchmark sequences'
    },
    'Benchmark': {
        'name': 'Benchmark',
        'path': SEQS_DIR / 'Benchmark',
        'description': 'General benchmark sequences'
    },
    'NUC': {
        'name': 'NUC',
        'path': SEQS_DIR / 'NUC',
        'description': 'Nucleotide sequences'
    },
    'PAM': {
        'name': 'PAM',
        'path': SEQS_DIR / 'PAM',
        'description': 'PAM sequences'
    }
}

# Execution settings
MAX_EXECUTION_TIMEOUT = 600  # 10 minutes in seconds
MAX_DIRECTORY_DEPTH = 5

# File extensions
FASTA_EXTENSIONS = ['.fasta', '.txt']
