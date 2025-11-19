"""
Alignment execution routes
"""

from flask import Blueprint, jsonify, request
import subprocess
from app.services.runner_service import run_alignment

run_bp = Blueprint('run', __name__)


@run_bp.route('/api/run', methods=['POST'])
def run_alignment_route():
    """Execute MSA alignment"""
    data = request.json

    algorithm = data.get('algorithm', 'msa_astar')
    binary_name = data.get('binary_name')
    file_path_str = data.get('file_path')
    cost_type = data.get('cost_type', 'PAM250')
    num_threads = data.get('num_threads', 4)
    verbose = data.get('verbose', False)

    if not file_path_str:
        return jsonify({'error': 'File path not provided'}), 400

    try:
        result = run_alignment(
            binary_name=binary_name,
            algorithm=algorithm,
            file_path=file_path_str,
            cost_type=cost_type,
            num_threads=num_threads,
            verbose=verbose
        )
        return jsonify(result)
    except FileNotFoundError as e:
        return jsonify({'error': str(e)}), 404
    except ValueError as e:
        return jsonify({'error': str(e)}), 400
    except subprocess.TimeoutExpired:
        return jsonify({
            'success': False,
            'error': 'Execution timeout (10 minutes)'
        }), 500
    except Exception as e:
        return jsonify({
            'success': False,
            'error': str(e)
        }), 500
