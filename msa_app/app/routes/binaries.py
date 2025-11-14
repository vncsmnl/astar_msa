"""
Binary management routes
"""

from flask import Blueprint, jsonify
from app.services.binaries_service import get_available_binaries

binaries_bp = Blueprint('binaries', __name__)


@binaries_bp.route('/api/binaries')
def get_binaries():
    """API endpoint to get available binary versions"""
    binaries = get_available_binaries()
    return jsonify(binaries)
