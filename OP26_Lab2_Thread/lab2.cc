#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <cmath>
#include <random>

using namespace std;

const int WIDTH = 100;
const int HEIGHT = 30;
const int DELAY_MS = 100;
const double V = 1.0;
const int N = 3;

mutex printMutex;

class BeeWorker {
private:
    double x, y;
    double startX, startY;

public:
    BeeWorker(double x, double y) : x(x), y(y), startX(x), startY(y) {}

    void move() {
        while (true) {
            moveTo(0, 0);
            moveTo(startX, startY);
        }
    }

    void moveTo(double targetX, double targetY) {
        while (abs(x - targetX) > 0.1 || abs(y - targetY) > 0.1) {
            double dx = targetX - x;
            double dy = targetY - y;
            double length = sqrt(dx * dx + dy * dy);

            x += V * dx / length;
            y += V * dy / length;

            {
                lock_guard<mutex> lock(printMutex);
                cout << "BeeWorker at (" << x << ", " << y << ")" << endl;
            }

            this_thread::sleep_for(chrono::milliseconds(DELAY_MS));
        }
    }
};

class Drone {
private:
    double x, y;
    double dx, dy;

public:
    Drone(double x, double y) : x(x), y(y) {
        randomDirection();
    }

    void randomDirection() {
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<> dist(-1.0, 1.0);

        dx = dist(gen);
        dy = dist(gen);
        double length = sqrt(dx * dx + dy * dy);
        dx /= length;
        dy /= length;
    }

    void move() {
        auto lastChange = chrono::steady_clock::now();

        while (true) {
            auto now = chrono::steady_clock::now();
            if (chrono::duration_cast<chrono::seconds>(now - lastChange).count() >= N) {
                randomDirection();
                lastChange = now;
            }

            x += dx * V;
            y += dy * V;

            if (x <= 0 || x >= WIDTH) dx *= -1;
            if (y <= 0 || y >= HEIGHT) dy *= -1;

            {
                lock_guard<mutex> lock(printMutex);
                cout << "Drone at (" << x << ", " << y << ")" << endl;
            }

            this_thread::sleep_for(chrono::milliseconds(DELAY_MS));
        }
    }
};

int main() {
    vector<thread> threads;

    BeeWorker bee1(50, 20);
    BeeWorker bee2(80, 10);

    Drone drone1(30, 15);
    Drone drone2(60, 5);

    threads.emplace_back(&BeeWorker::move, &bee1);
    threads.emplace_back(&BeeWorker::move, &bee2);
    threads.emplace_back(&Drone::move, &drone1);
    threads.emplace_back(&Drone::move, &drone2);

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
