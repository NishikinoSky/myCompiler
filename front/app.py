from flask import Flask
from flask import request
from flask import jsonify, make_response
from flask_cors import CORS

app = Flask(__name__)
CORS(app, resources={r"/*"})

@app.route("/run", methods=['POST'])
async def postRun():
    response = await handleRun(request)
    return response


async def handleRun(request):
    res = request.values.get('source')
    print(res)
    return make_response(jsonify({'data': "get it!\n"+res, 'status': 200, 'error': None}), 200)


app.run(host='0.0.0.0', port=81)
