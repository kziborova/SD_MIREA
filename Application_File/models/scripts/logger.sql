CREATE TYPE operation_type AS ENUM ('create', 'modify', 'delete');

CREATE TABLE IF NOT EXISTS Operations (
    id SERIAL PRIMARY KEY,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    type operation_type NOT NULL,
    file_id INTEGER REFERENCES Files(id) ON DELETE SET NULL,
    user_id UUID REFERENCES users(id) ON DELETE SET NULL
);

SELECT * FROM Operations;