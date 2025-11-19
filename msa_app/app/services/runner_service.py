"""
Alignment execution service
Handles running MSA algorithms
"""

import subprocess
import time
import json
from datetime import datetime
from pathlib import Path
from app.config import RESULTS_DIR, MAX_EXECUTION_TIMEOUT
from app.services.binaries_service import get_binary_path

# Create results directory if it doesn't exist
RESULTS_DIR.mkdir(exist_ok=True)


def run_alignment(binary_name, algorithm, file_path, cost_type='PAM250', num_threads=4, verbose=False):
    """
    Execute MSA alignment

    Args:
        verbose: If True, include -l flag for verbose output

    Returns:
        dict: Result information including execution time, output, etc.
    """
    # Get binary path and capabilities
    binary_path, supports_threads = get_binary_path(
        binary_name=binary_name,
        algorithm=algorithm
    )

    # Validate input file
    file_path = Path(file_path)
    if not file_path.exists():
        raise FileNotFoundError(f'File not found: {file_path}')

    # Create unique result identifier
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    binary_version = binary_path.name
    result_id = f"{binary_version}_{timestamp}"
    output_file = RESULTS_DIR / f"{result_id}.fasta"
    log_file = RESULTS_DIR / f"{result_id}.log"
    verbose_log_file = RESULTS_DIR / f"{result_id}_verbose.txt" if verbose else None

    # Build command
    cmd = [str(binary_path)]

    # Add cost type
    cmd.extend(['-c', cost_type])

    # Add output file
    cmd.extend(['-f', str(output_file)])

    # Add threads only if binary supports it
    if supports_threads:
        cmd.extend(['-t', str(num_threads)])

    # Add verbose flag and log file if requested
    if verbose and verbose_log_file:
        cmd.extend(['-l', str(verbose_log_file)])

    # Add input file
    cmd.append(str(file_path))

    # Execute
    start_time = time.time()
    result = subprocess.run(
        cmd,
        capture_output=True,
        text=True,
        timeout=MAX_EXECUTION_TIMEOUT
    )
    execution_time = time.time() - start_time

    # Save log
    log_content = {
        'binary': binary_version,
        'command': ' '.join(cmd),
        'execution_time': execution_time,
        'return_code': result.returncode,
        'stdout': result.stdout,
        'stderr': result.stderr,
        'timestamp': timestamp,
        'input_file': str(file_path),
        'cost_type': cost_type,
        'num_threads': num_threads if supports_threads else None,
        'supports_threads': supports_threads,
        'verbose': verbose,
        'verbose_log_file': str(verbose_log_file) if verbose_log_file else None
    }

    with open(log_file, 'w') as f:
        json.dump(log_content, f, indent=2)

    # Read output if successful
    output_content = ""
    if output_file.exists():
        with open(output_file, 'r') as f:
            output_content = f.read()

    # Read verbose log if it exists
    verbose_log_content = ""
    if verbose_log_file and verbose_log_file.exists():
        with open(verbose_log_file, 'r') as f:
            verbose_log_content = f.read()

    return {
        'success': True,
        'result_id': result_id,
        'binary': binary_version,
        'execution_time': execution_time,
        'stdout': result.stdout,
        'stderr': result.stderr,
        'output_file': str(output_file),
        'output_content': output_content,
        'return_code': result.returncode,
        'verbose': verbose,
        'verbose_log_content': verbose_log_content
    }
