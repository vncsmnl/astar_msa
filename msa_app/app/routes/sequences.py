"""
Sequence management routes
"""

from flask import Blueprint, jsonify, request
from pathlib import Path
from app.services.sequences_service import scan_all_sequences
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

    if not file_path_str:
        return jsonify({'error': 'File path not provided'}), 400

    file_path = Path(file_path_str)

    if not file_path.exists():
        return jsonify({'error': 'File not found'}), 404

    # Parse FASTA file
    sequences = parse_fasta_file(file_path)

    # Analyze sequences
    overall_type = analyze_sequences(sequences)

    return jsonify({
        'num_sequences': len(sequences),
        'sequences': sequences,
        'sequence_type': overall_type,
        'file_path': str(file_path)
    })
