import tensorflow as tf
import numpy as np
import matplotlib.pyplot as plt

# load and prepare the data
mnist_data = tf.keras.datasets.mnist
(train_images, train_labels), (test_images, test_labels) = mnist_data.load_data()
train_images = train_images / 255
test_images = test_images / 255
train_images = train_images.reshape(train_images.shape[0], 28, 28, 1)
test_images  = test_images.reshape(test_images.shape[0], 28, 28, 1)

# put together the network and compile it
model = tf.keras.Sequential([
	tf.keras.layers.Conv2D(20, (5,5), activation=tf.nn.relu, input_shape=(28,28,1)),
	tf.keras.layers.MaxPool2D(2,2),
	tf.keras.layers.Conv2D(64, (3,3), activation=tf.nn.relu, input_shape=(12,12,20)),
	tf.keras.layers.MaxPool2D(2,2),
	tf.keras.layers.Flatten(input_shape=(64,5,5)),
	tf.keras.layers.Dense(100,activation=tf.nn.relu),
	tf.keras.layers.Dense(10,activation=tf.nn.relu),
	tf.keras.layers.Softmax()
])
model.compile(optimizer="adam", loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True), metrics=['accuracy'])

# now train it ....
#model.fit(train_images, train_labels, epochs=2)

# and save it ....
#model.save("test")

# ... alteratively load it from a previous run
model = tf.keras.models.load_model('test')

# make a couple of predictions
probability_model = tf.keras.Sequential([model, tf.keras.layers.Softmax()])
predictions = probability_model.predict(test_images)

# inspect the result
plt.figure(figsize=(18,5))
for i in range(100):
	plt.subplot(5,20,i+1)
	plt.xticks([])
	plt.yticks([])
	plt.grid(False)
	plt.imshow(test_images[i], cmap=plt.cm.binary)
	plt.xlabel(np.argmax(predictions[i]))
plt.show()

