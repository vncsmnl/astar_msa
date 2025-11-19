"""
Results management service
Handles result storage and retrieval
"""

import json
from pathlib import Path
from app.config import RESULTS_DIR


def list_results():
    """List all previous results"""
    results = []

    for log_file in sorted(RESULTS_DIR.glob('*.log'), reverse=True):
        # Skip verbose log files
        if log_file.stem.endswith('_verbose'):
            continue

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

    return results


def get_result(result_id):
    """Get specific result details"""
    log_file = RESULTS_DIR / f"{result_id}.log"
    output_file = RESULTS_DIR / f"{result_id}.fasta"
    verbose_log_file = RESULTS_DIR / f"{result_id}_verbose.log"

    if not log_file.exists():
        raise FileNotFoundError('Result not found')

    with open(log_file, 'r') as f:
        log_data = json.load(f)

    output_content = ""
    if output_file.exists():
        with open(output_file, 'r') as f:
            output_content = f.read()

    verbose_log_content = ""
    if verbose_log_file.exists():
        with open(verbose_log_file, 'r') as f:
            verbose_log_content = f.read()

    log_data['output_content'] = output_content
    log_data['verbose_log_content'] = verbose_log_content
    log_data['result_id'] = result_id

    return log_data


def get_result_file_path(result_id):
    """Get the file path for a result"""
    output_file = RESULTS_DIR / f"{result_id}.fasta"

    if not output_file.exists():
        raise FileNotFoundError('File not found')

    return output_file
