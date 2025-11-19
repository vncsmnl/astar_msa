"""
Sequence management routes
"""

from flask import Blueprint, jsonify, request
from pathlib import Path
from app.services.sequences_service import scan_all_sequences
from app.services.binaries_service import get_binary_path
from app.utils.fasta import parse_fasta_file, analyze_sequences

sequences_bp = Blueprint('sequences', __name__)


@sequences_bp.route('/api/sequences')
def get_sequences():
    """API endpoint to get available sequences"""
    all_sequences = scan_all_sequences()
    return jsonify(all_sequences)


@sequences_bp.route('/api/sequence_info', methods=['POST'])
def get_sequence_info():
    """Get information about a sequence file"""
    data = request.json
    file_path_str = data.get('file_path')
    algorithm = data.get('algorithm', 'msa_astar')
    binary_name = data.get('binary_name')
    cost_type = data.get('cost_type', 'PAM250')
    num_threads = data.get('num_threads', 4)
    verbose = data.get('verbose', False)

    if not file_path_str:
        return jsonify({'error': 'File path not provided'}), 400

    file_path = Path(file_path_str)

    if not file_path.exists():
        return jsonify({'error': 'File not found'}), 404

    # Parse FASTA file
    sequences = parse_fasta_file(file_path)

    # Analyze sequences
    overall_type = analyze_sequences(sequences)

    # Build example command
    command = None
    try:
        if binary_name:
            binary_path, supports_threads = get_binary_path(
                binary_name=binary_name,
                algorithm=algorithm
            )
            cmd_parts = [str(binary_path), '-c', cost_type]
            if supports_threads:
                cmd_parts.extend(['-t', str(num_threads)])
            if verbose:
                cmd_parts.append('-l')
            cmd_parts.append(str(file_path))
            command = ' '.join(cmd_parts)
    except Exception:
        # If binary not selected, show generic command
        pass

    return jsonify({
        'num_sequences': len(sequences),
        'sequences': sequences,
        'sequence_type': overall_type,
        'file_path': str(file_path),
        'command': command
    })
