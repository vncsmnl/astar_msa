#!/usr/bin/env python3
"""
Flask for MSA A-Star and PA-Star
Manage executions and select BALIBASE test sequences
"""

from flask import Flask, render_template, request, jsonify, send_file
import os
import subprocess
import time
from datetime import datetime
import json
from pathlib import Path

app = Flask(__name__)
app.config['SECRET_KEY'] = 'msa-astar-pastar-secret-key'

# Base directory
BASE_DIR = Path(__file__).parent.parent.absolute()
BIN_DIR = BASE_DIR / 'bin'
SEQS_DIR = BASE_DIR / 'seqs'
RESULTS_DIR = Path(__file__).parent / 'results'

# Create results directory if it doesn't exist
RESULTS_DIR.mkdir(exist_ok=True)

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


def scan_directory_recursively(directory, max_depth=5, current_depth=0):
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
    fasta_files = [item.name for item in items if item.is_file() and item.suffix in ['.fasta', '.txt']]

    # If there are FASTA files in this directory, add them
    if fasta_files:
        structure['_files'] = fasta_files

    # Recursively scan subdirectories
    for subdir in subdirs:
        subdir_structure = scan_directory_recursively(subdir, max_depth, current_depth + 1)
        if subdir_structure:  # Only add if not empty
            structure[subdir.name] = subdir_structure

    return structure


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
        categories = scan_directory_recursively(source_path, max_depth=5)

        if categories:
            source_structure['categories'] = categories
            all_sequences[source_key] = source_structure

    return all_sequences


@app.route('/')
def index():
    """Main page"""
    all_sequences = scan_all_sequences()

    return render_template('index.html',
                           all_sequences=all_sequences)


@app.route('/api/sequences')
def get_sequences():
    """API endpoint to get available sequences"""
    all_sequences = scan_all_sequences()

    return jsonify(all_sequences)


@app.route('/api/run', methods=['POST'])
def run_alignment():
    """Execute MSA alignment"""
    data = request.json

    algorithm = data.get('algorithm', 'msa_astar')
    sequence_file = data.get('sequence_file')
    file_path_str = data.get('file_path')  # Full path from frontend
    cost_type = data.get('cost_type', 'PAM250')
    num_threads = data.get('num_threads', 4)

    # Validate algorithm
    if algorithm not in ['msa_astar', 'msa_pastar']:
        return jsonify({'error': 'Invalid algorithm'}), 400

    # Build file path
    if file_path_str:
        file_path = Path(file_path_str)
    else:
        return jsonify({'error': 'File path not provided'}), 400

    if not file_path.exists():
        return jsonify({'error': f'File not found: {file_path}'}), 404

    # Create unique result identifier
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    result_id = f"{algorithm}_{timestamp}"
    output_file = RESULTS_DIR / f"{result_id}.fasta"
    log_file = RESULTS_DIR / f"{result_id}.log"

    # Build command
    binary = BIN_DIR / algorithm
    cmd = [str(binary)]

    # Add cost type
    cmd.extend(['-c', cost_type])

    # Add output file
    cmd.extend(['-f', str(output_file)])

    # Add threads for parallel version
    if algorithm == 'msa_pastar':
        cmd.extend(['-t', str(num_threads)])

    # Add input file
    cmd.append(str(file_path))

    # Execute
    try:
        start_time = time.time()
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=300  # 5 minutes timeout
        )
        execution_time = time.time() - start_time

        # Save log
        log_content = {
            'command': ' '.join(cmd),
            'execution_time': execution_time,
            'return_code': result.returncode,
            'stdout': result.stdout,
            'stderr': result.stderr,
            'timestamp': timestamp
        }

        with open(log_file, 'w') as f:
            json.dump(log_content, f, indent=2)

        # Read output if successful
        output_content = ""
        if output_file.exists():
            with open(output_file, 'r') as f:
                output_content = f.read()

        return jsonify({
            'success': True,
            'result_id': result_id,
            'execution_time': execution_time,
            'stdout': result.stdout,
            'stderr': result.stderr,
            'output_file': str(output_file),
            'output_content': output_content,
            'return_code': result.returncode
        })

    except subprocess.TimeoutExpired:
        return jsonify({
            'success': False,
            'error': 'Execution timeout (5 minutes)'
        }), 500
    except Exception as e:
        return jsonify({
            'success': False,
            'error': str(e)
        }), 500


@app.route('/api/results')
def list_results():
    """List all previous results"""
    results = []

    for log_file in sorted(RESULTS_DIR.glob('*.log'), reverse=True):
        try:
            with open(log_file, 'r') as f:
                log_data = json.load(f)
                results.append({
                    'id': log_file.stem,
                    'timestamp': log_data.get('timestamp'),
                    'execution_time': log_data.get('execution_time'),
                    'return_code': log_data.get('return_code')
                })
        except:
            pass

    return jsonify(results)


@app.route('/api/result/<result_id>')
def get_result(result_id):
    """Get specific result details"""
    log_file = RESULTS_DIR / f"{result_id}.log"
    output_file = RESULTS_DIR / f"{result_id}.fasta"

    if not log_file.exists():
        return jsonify({'error': 'Result not found'}), 404

    with open(log_file, 'r') as f:
        log_data = json.load(f)

    output_content = ""
    if output_file.exists():
        with open(output_file, 'r') as f:
            output_content = f.read()

    log_data['output_content'] = output_content
    return jsonify(log_data)


@app.route('/api/download/<result_id>')
def download_result(result_id):
    """Download result file"""
    output_file = RESULTS_DIR / f"{result_id}.fasta"

    if not output_file.exists():
        return jsonify({'error': 'File not found'}), 404

    return send_file(output_file, as_attachment=True)


@app.route('/api/sequence_info', methods=['POST'])
def get_sequence_info():
    """Get information about a sequence file"""
    data = request.json
    file_path_str = data.get('file_path')  # Full path from frontend

    # Build file path
    if file_path_str:
        file_path = Path(file_path_str)
    else:
        return jsonify({'error': 'File path not provided'}), 400

    if not file_path.exists():
        return jsonify({'error': 'File not found'}), 404

    # Read and parse FASTA file
    sequences = []
    current_seq = None

    with open(file_path, 'r') as f:
        for line in f:
            line = line.strip()
            if line.startswith('>'):
                if current_seq:
                    sequences.append(current_seq)
                current_seq = {
                    'header': line[1:],
                    'sequence': ''
                }
            elif current_seq is not None:
                current_seq['sequence'] += line

        if current_seq:
            sequences.append(current_seq)

    # Calculate statistics
    for seq in sequences:
        seq['length'] = len(seq['sequence'])

    return jsonify({
        'num_sequences': len(sequences),
        'sequences': sequences,
        'file_path': str(file_path)
    })


if __name__ == '__main__':
    print("=" * 60)
    print("MSA A-Star / PA-Star Flask")
    print("=" * 60)
    print(f"Base directory: {BASE_DIR}")
    print(f"Binaries: {BIN_DIR}")
    print(f"Sequences: {SEQS_DIR}")
    print("Sources available:")
    for source_key, source_info in SOURCES.items():
        if source_info['path'].exists():
            print(f"  - {source_info['name']}: {source_info['path']}")
    print(f"Results: {RESULTS_DIR}")
    print("=" * 60)
    print("Starting server on http://localhost:5000")
    print("=" * 60)

    app.run(debug=True, host='0.0.0.0', port=5000)
