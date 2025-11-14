"""
Flask application factory
Initializes and configures the Flask application
"""

from flask import Flask
from app.config import SECRET_KEY


def create_app():
    """Create and configure the Flask application"""
    app = Flask(__name__)
    app.config['SECRET_KEY'] = SECRET_KEY

    # Register blueprints
    from app.routes import main_bp
    from app.routes.binaries import binaries_bp
    from app.routes.sequences import sequences_bp
    from app.routes.run import run_bp
    from app.routes.results import results_bp

    app.register_blueprint(main_bp)
    app.register_blueprint(binaries_bp)
    app.register_blueprint(sequences_bp)
    app.register_blueprint(run_bp)
    app.register_blueprint(results_bp)

    return app
