"""
Routes initialization
Imports all blueprints for registration
"""

from flask import Blueprint

# Create a main blueprint for the index route
main_bp = Blueprint('main', __name__)

@main_bp.route('/')
def index():
    """Main page"""
    from flask import render_template
    from app.services.sequences_service import scan_all_sequences
    
    all_sequences = scan_all_sequences()
    return render_template('index.html', all_sequences=all_sequences)
