#!/usr/bin/env python3
"""
Flask for MSA A-Star and PA-Star
Main entry point for the application

Author: Vinícius Manoel
Copyright: MIT License
"""

from app import create_app
from app.config import BASE_DIR, BIN_DIR, SEQS_DIR, RESULTS_DIR, SOURCES
from app.services.binaries_service import get_available_binaries


if __name__ == '__main__':
    # Create the Flask app
    app = create_app()

    print("=" * 60)
    print("MSA A-Star / PA-Star Flask")
    print("=" * 60)
    print(f"Base directory: {BASE_DIR}")
    print(f"Binaries: {BIN_DIR}")

    # List available binaries
    binaries = get_available_binaries()
    print("\nAvailable binaries:")
    print("  A-Star versions:")
    for binary in binaries['astar']:
        print(f"    - {binary['name']}")
    print("  PA-Star versions:")
    for binary in binaries['pastar']:
        print(f"    - {binary['name']}")

    print(f"\nSequences: {SEQS_DIR}")
    print("Sources available:")
    for source_key, source_info in SOURCES.items():
        if source_info['path'].exists():
            print(f"  - {source_info['name']}: {source_info['path']}")
    print(f"\nResults: {RESULTS_DIR}")
    print("=" * 60)
    print("Starting server on http://localhost:5000")
    print("=" * 60)

    app.run(debug=True, host='0.0.0.0', port=5000)
