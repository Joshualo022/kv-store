import threading
import unittest
from client import connect, command, unique_key

NUM_CLIENTS = 5
OPS_PER_CLIENT = 500


def worker(prefix, client_id, errors):
    try:
        sock = connect()
        try:
            for i in range(OPS_PER_CLIENT):
                command(sock, f"SET {prefix}_c{client_id}_k{i} v{client_id}_{i}")
            for i in range(OPS_PER_CLIENT):
                got = command(sock, f"GET {prefix}_c{client_id}_k{i}")
                if got != f"v{client_id}_{i}":
                    errors.append((client_id, i, got))
        finally:
            sock.close()
    except Exception as e:
        errors.append((client_id, "exception", repr(e)))


class TestConcurrency(unittest.TestCase):
    def test_parallel_clients_keep_their_values(self):
        # Fresh keys force new inserts and resizes, which is where races happen
        prefix = unique_key("conc")
        errors = []
        threads = [threading.Thread(target=worker, args=(prefix, c, errors))
                   for c in range(NUM_CLIENTS)]
        for t in threads:
            t.start()
        for t in threads:
            t.join()
        self.assertEqual(errors, [], f"{len(errors)} errors, first few: {errors[:5]}")


if __name__ == "__main__":
    unittest.main()