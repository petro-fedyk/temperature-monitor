from flask import Flask
from flask_sqlalchemy import SQLAlchemy
from flask_restful import Resource, Api, reqparse, fields, marshal_with
from datetime import datetime
from flask_cors import CORS


app = Flask(__name__)
CORS(app)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///database.db'
db = SQLAlchemy(app)
api = Api(app)

class FridgeTag(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    time = db.Column(db.DateTime)
    temperatureC = db.Column(db.Float)
    maxTemperature = db.Column(db.Float)
    minTemperature = db.Column(db.Float)
    isMaxAlarm = db.Column(db.Boolean)
    isMinAlarm = db.Column(db.Boolean)

    def __repr__(self):
        return f"FridgeTag(time={self.time}, temperatureC={self.temperatureC}, maxTemperature={self.maxTemperature}, minTemperature={self.minTemperature})"

# Кастомна функція для перетворення рядка у datetime
def parse_datetime(value):
    try:
        return datetime.strptime(value, "%Y.%m.%d.%H:%M")
    except ValueError:
        raise ValueError("time must be in format YYYY.MM.DD.HH:MM")

tag_args = reqparse.RequestParser()
tag_args.add_argument('time', type=parse_datetime, required=True)
tag_args.add_argument('temperatureC', type=float, required=True)
tag_args.add_argument('maxTemperature', type=float, required=True)
tag_args.add_argument('minTemperature', type=float, required=True)
tag_args.add_argument('isMaxAlarm', type=bool, required=True)
tag_args.add_argument('isMinAlarm', type=bool, required=True)

tag_fields = {
    'id': fields.Integer,
    'time': fields.String,
    'temperatureC': fields.Float,
    'maxTemperature': fields.Float,
    'minTemperature': fields.Float,
    'isMaxAlarm': fields.Boolean,
    'isMinAlarm': fields.Boolean
}

class Tag(Resource):
    @marshal_with(tag_fields)
    def get(self):
        return FridgeTag.query.all()

    @marshal_with(tag_fields)
    def post(self):
        args = tag_args.parse_args()
        tag_data = FridgeTag(
            time=args['time'],
            temperatureC=args['temperatureC'],
            maxTemperature=args['maxTemperature'],
            minTemperature=args['minTemperature'],
            isMaxAlarm=args['isMaxAlarm'],
            isMinAlarm=args['isMinAlarm']
        )
        db.session.add(tag_data)
        db.session.commit()
        return FridgeTag.query.all(), 201
    
class TagById(Resource):
    @marshal_with(tag_fields)
    def get(self, id):
        tag_data = FridgeTag.query.filter_by(id=id).first()
        if not tag_data:
            abort(404, "Data not found")
        return tag_data

api.add_resource(Tag, '/api/tag_data/')
api.add_resource(TagById, '/api/tag_data/<int:id>')


@app.route('/')
def home():
    return '<h1>Flask REST API</h1>'

if __name__ == '__main__':
    with app.app_context():
        db.create_all()
    app.run(debug=True, host="0.0.0.0")

