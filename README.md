# Database structure

Chat room database contains 3 tables:
* users (id, account, password, username, last_logined)
* messages (id, from_id, to_id, group_id, sending_time, message)
* groups (id, user_id, name)