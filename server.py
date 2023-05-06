from flask import Flask, render_template_string, request

app = Flask(__name__)
last_L_value = "no data"
last_R_value = "no data"

@app.route("/")
def index():
    global last_L_value, last_R_value
    var = request.args.get("var") # changed from "L" and "R"
    if var:
        device, value = var.split("_") # extract device ID and value
        if device == "L":
            last_L_value = value
        elif device == "R":
            last_R_value = value
    template = """
    <html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <meta http-equiv="refresh" content="0.5">
        <style>
            body {
                font-size: 24px;
            }
        </style>
    </head>  

    <body>
        <h1>Bench Buddy</h1>
        <p>Left: {{ last_L_value }}</p>
        <p>Right: {{ last_R_value }}</p>
    </body>
    </html>
    """
    return render_template_string(template, last_L_value=last_L_value, last_R_value=last_R_value)



