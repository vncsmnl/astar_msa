"""
Results management routes
"""

from flask import Blueprint, jsonify, send_file
from app.services.results_service import list_results, get_result, get_result_file_path

results_bp = Blueprint('results', __name__)


@results_bp.route('/api/results')
def list_results_route():
    """List all previous results"""
    results = list_results()
    return jsonify(results)


@results_bp.route('/api/result/<result_id>')
def get_result_route(result_id):
    """Get specific result details"""
    try:
        result = get_result(result_id)
        return jsonify(result)
    except FileNotFoundError:
        return jsonify({'error': 'Result not found'}), 404


@results_bp.route('/api/download/<result_id>')
def download_result(result_id):
    """Download result file"""
    try:
        output_file = get_result_file_path(result_id)
        return send_file(output_file, as_attachment=True)
    except FileNotFoundError:
        return jsonify({'error': 'File not found'}), 404
