import numpy as np
import matplotlib.pyplot as plt 



def generate_fibonacci_sphere(num_points, radius):
    increment = np.pi * (3.0 - np.sqrt(5.0))
    offset = 2.0 / num_points
    points = np.zeros((num_points, 3))
    for i in range(num_points):
        y = ((i * offset) - 1.0) + offset / 2.0
        r = np.sqrt(1.0 - y * y)
        phi = i * increment
        x = r * np.cos(phi)
        z = r * np.sin(phi)
        points[i] = np.array([x * radius, y * radius, z * radius])
    return points

radius = 166.277
num_points = 100

points = generate_fibonacci_sphere(num_points, radius)

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.scatter(points[:,0], points[:,1], points[:,2])
plt.show()