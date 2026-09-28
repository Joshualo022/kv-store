import struct
import unittest
from client import connect, command, recv_response, unique_key


class TestErrors(unittest.TestCase):
    def setUp(self):
        self.sock = connect()

    def tearDown(self):
        self.sock.close()

    def test_set_missing_value(self):
        self.assertEqual(command(self.sock, "SET onlykey"),
                         "ERROR: SET requires a key and value")

    def test_set_missing_key(self):
        self.assertEqual(command(self.sock, "SET"),
                         "ERROR: SET requires a key and value")

    def test_get_missing_key(self):
        self.assertEqual(command(self.sock, "GET"), "ERROR: GET requires key")

    def test_del_missing_key(self):
        self.assertEqual(command(self.sock, "DEL"), "ERROR: DEL requires key")

    def test_unknown_command(self):
        self.assertEqual(command(self.sock, "FOO bar baz"), "ERROR: Unknown Command")

    def test_whitespace_only(self):
        # Requires the cmd->command == NULL guard in handle_client
        self.assertEqual(command(self.sock, " "), "ERROR: Empty command")

    def test_connection_survives_bad_commands(self):
        # A bad command should get an error, not kill the connection
        command(self.sock, "SET")
        command(self.sock, "FOO")
        key = unique_key("survive")
        self.assertEqual(command(self.sock, f"SET {key} ok"), "SET operation OK")
        self.assertEqual(command(self.sock, f"GET {key}"), "ok")

    def test_oversized_message_closes_connection(self):
        # Claim a 5000-byte message; the server should refuse and disconnect
        self.sock.sendall(struct.pack(">I", 5000))
        with self.assertRaises(ConnectionError):
            recv_response(self.sock)

        # The server itself should still accept new clients
        other = connect()
        try:
            key = unique_key("after_oversize")
            self.assertEqual(command(other, f"SET {key} fine"), "SET operation OK")
        finally:
            other.close()


if __name__ == "__main__":
    unittest.main()