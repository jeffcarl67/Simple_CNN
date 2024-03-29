#include "NN.hpp"
#include "BinaryStream.hpp"
using namespace std;

NN::NN(const vector<int> shape, double eps, int N, double learning_rate): eps(eps), N(N) {
	// hidden and output layer, input layer is not used here
	for (int i = 1; i < shape.size(); ++i) {
		vector<Neuron> layer;
		for (int j = 0; j < shape[i]; ++j)
			layer.push_back(Neuron(shape[i-1], learning_rate)); // number of prev layer neurons
		net.push_back(layer);
	}
}

NN::NN(const vector<vector<double> > shape_learning_rate, double eps, int N): eps(eps), N(N) {
	// hidden and output layer, input layer is not used here
	for (int i = 1; i < shape_learning_rate.size(); ++i) {
		vector<Neuron> layer;
		for (int j = 0; j < (int)shape_learning_rate[i][0]; ++j)
			// number of prev layer neurons
			layer.push_back(Neuron((int)shape_learning_rate[i-1][0], shape_learning_rate[i][1]));
		net.push_back(layer);
	}
}

NN::NN(Layers weights, double eps, int N, double learning_rate): eps(eps), N(N) {
	// input, hidden and output layer
	// input layer should only have a bias for each neuron
	for (int i = 0; i < weights.size(); ++i) {
		vector<Neuron> layer;
		for (int j = 0; j < weights[i].size(); ++j)
			layer.push_back(Neuron(weights[i][j], learning_rate));
		net.push_back(layer);
	}
}

vector<double> NN::getOutput(int layer){
	vector<double> output;
	for (int j = 0; j < net[layer].size(); ++j) {
		output.push_back(net[layer][j].getOutput());
	}
	return output;
}

void NN::forward(const vector<double>& input) {
	if (net.size() > 0 && net[0].size() > 0 && input.size() != net[0][0].size() - 1)
		throw "NN input size error";
	// assign input value
	for (int j = 0; j < net[0].size(); ++j)
		net[0][j].cal(input);
	// feed dorward
	for (int i = 1; i < net.size(); ++i) {
		for (int j = 0; j < net[i].size(); ++j) {
			net[i][j].cal(getOutput(i - 1));
		}
	}
	last_input = input;
}

void NN::backProp(const vector<double>& expect_output) {
	// calculate output layer gradient
	vector<Neuron> &output_layer = net[net.size() - 1];
	if (expect_output.size() != output_layer.size()) throw "NN expect output size error";
	for (int j = 0; j < output_layer.size(); ++j) {
		output_layer[j].calOutputGradient(expect_output[j]);
	}

	// calculate hidden layer gradient
	for (int i = net.size() - 2; i >= 0; --i) {
		for (int j = 0; j < net[i].size(); ++j) {
			net[i][j].calHiddenGradient(net[i+1], j);
		}
	}

	// update all weights
	for (int i = net.size() - 1; i > 0; --i) { // hidden layers
		for (int j = 0; j < net[i].size(); ++j) {
			net[i][j].update(getOutput(i - 1));
		}
	}
	for (int j = 0; j < net[0].size(); ++j) {
		net[0][j].update(last_input);
	}
}

vector<double> NN::getResult(const vector<double>& input) {
	forward(input);
	return getOutput(net.size() - 1);
}

double NN::calStandardError() {
	double error = 0;
	double count = 0;
	for (int i = 0; i < net.size(); ++i) {
		for (int j = 0; j < net[i].size(); ++j) {
			count += net[i][j].size();
			error += net[i][j].calSquareError();
		}
	}
	return sqrt(error / count);
}

double NN::sample_error(const Records& data) {
	double err_num = 0;
	for (int i = 0; i < data.size(); ++i) {
		vector<double> res = getResult(data[i].data);
		int ans = argmax(res);
		if (ans != data[i].id) err_num++;
	}
	return err_num / data.size();
}

double NN::train(Records train_data, bool show) {
	// eps == 0: just depend on N
	// N == 0: just depend on eps
	int count = 0;
	int iteration = 0;
	double weight_err;
	while (iteration < N || N == 0) {
		forward(train_data[count].data);
		backProp(train_data[count].output);
		count++;
		if (count % train_data.size() == 0) {
			weight_err = calStandardError();
			if (weight_err < eps) break;
			count %= train_data.size();
			iteration++;
			if (show) cout << "iteration = " << iteration << ", weight error = " << weight_err << ", Ein = " << sample_error(train_data) << endl;
			train_data.shuffle();
		}
	}

	return sample_error(train_data);
}

double NN::test(Records test_data) {
	return sample_error(test_data);
}

ostream& operator<<(ostream& os, const NN& nn) {
	// { double eps, int N, int layer_num, { int neuron_num, Neuron[neuron_num] }[layer_num] }
	BinaryStream bs;
	// write obj to stream
	bs.writeDouble(os, nn.eps);
	bs.writeInt(os, nn.N);
	bs.writeInt(os, nn.net.size());
	for (int i = 0; i < nn.net.size(); i++) {
		bs.writeInt(os, nn.net[i].size());
		for (int j = 0; j < nn.net[i].size(); j++)
			os << nn.net[i][j];
	}
	return os;
}

istream& operator>>(istream& is, NN& nn) {
	// { double eps, int N, int layer_num, { int neuron_num, Neuron[neuron_num] }[layer_num] }
	BinaryStream bs;
	// read obj from stream
	nn.eps = bs.readDouble(is);
	nn.N = bs.readInt(is);
	int layer_num = bs.readInt(is);
	for (int i = 0; i < layer_num; i++) {
		vector<Neuron> layer;
		int neuron_num = bs.readInt(is);
		for (int j = 0; j < neuron_num; j++) {
			Neuron n;
			is >> n;
			layer.push_back(n);
		}
		nn.net.push_back(layer);
	}
	return is;
}

void NN::print() {
	cout << "NN:" << endl;
	cout << "eps: " << eps << endl;
	cout << "N: " << N << endl;
	for (int i = 0; i < net.size(); ++i) {
		cout << "Layer " << i + 1 << ": " << endl;
		for (int j = 0; j < net[i].size(); ++j)
			net[i][j].print();
		cout << endl;
	}
}