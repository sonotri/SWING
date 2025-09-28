from flask import Flask, render_template, request, redirect, url_for
import pymysql

app = Flask(__name__)

# MySQL 연결 정보
db_config = {
    'host': 'localhost',
    'user': 'root',      
    'password': '123456',
    'database': 'hacking_camp',
    'cursorclass': pymysql.cursors.DictCursor
}

# DB 초기화
def init_db():
    conn = pymysql.connect(**db_config)
    c = conn.cursor()
    c.execute('''
        CREATE TABLE IF NOT EXISTS items (
            id INT AUTO_INCREMENT PRIMARY KEY,
            name VARCHAR(100) NOT NULL,
            description TEXT
        )
    ''')
    conn.commit()
    conn.close()

init_db()

# 전체 목록 조회
@app.route('/')
def index():
    conn = pymysql.connect(**db_config)
    c = conn.cursor()
    c.execute("SELECT * FROM items")
    items = c.fetchall()
    conn.close()
    return render_template('index.html', items=items)

# 데이터 추가
@app.route('/add', methods=['GET', 'POST'])
def add():
    if request.method == 'POST':
        name = request.form['name']
        description = request.form['description']
        conn = pymysql.connect(**db_config)
        c = conn.cursor()
        c.execute("INSERT INTO items (name, description) VALUES (%s, %s)", (name, description))
        conn.commit()
        conn.close()
        return redirect(url_for('index'))
    return render_template('add.html')

# 데이터 수정
@app.route('/edit/<int:item_id>', methods=['GET', 'POST'], strict_slashes=False)
def edit(item_id):
    conn = pymysql.connect(**db_config)
    c = conn.cursor()
    c.execute("SELECT * FROM items WHERE id=%s", (item_id,))
    item = c.fetchone()
    if item is None:
        conn.close()
        return "Item not found", 404

    if request.method == 'POST':
        name = request.form['name']
        description = request.form['description']
        c.execute("UPDATE items SET name=%s, description=%s WHERE id=%s", (name, description, item_id))
        conn.commit()
        conn.close()
        return redirect(url_for('index'))

    conn.close()
    return render_template('edit.html', item=item)

# 데이터 삭제
@app.route('/delete/<int:item_id>', strict_slashes=False)
def delete(item_id):
    conn = pymysql.connect(**db_config)
    c = conn.cursor()
    c.execute("DELETE FROM items WHERE id=%s", (item_id,))
    conn.commit()
    conn.close()
    return redirect(url_for('index'))

if __name__ == '__main__':
    app.run(debug=True)

