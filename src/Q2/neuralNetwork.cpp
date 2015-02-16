#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
using namespace std;

struct Node {
  double input;
  double output;
  double delta;
};

struct Layer {
  Node* nodes;
  double **weight;
  double **deltaWeight;
  Layer *next;
  double output;
};

struct Sample {
  double x1;
  int t1;
  int t2;
};

double computeOutput(double input) {
  return 1/(1+exp(-input));
}

int main(int argc, char* argv[]) {

  // construct networks
  Layer network;
  network.nodes = new Node[2];
  network.weight = new double *[2];
  network.weight[0] = new double[2];
  network.weight[1] = new double[2];
  network.deltaWeight = new double *[2];
  network.deltaWeight[0] = new double[2];
  network.deltaWeight[1] = new double[2];
  network.next = new Layer;
  network.next->nodes = new Node[2];
  network.next->weight = new double *[2];
  network.next->weight[0] = new double[2];
  network.next->weight[1] = new double[2];
  network.next->deltaWeight = new double *[2];
  network.next->deltaWeight[0] = new double[2];
  network.next->deltaWeight[1] = new double[2];
  network.next->next = new Layer;
  network.next->next->nodes = new Node[2];
  double learningRate = 0.5;

  // import training set
  ifstream trainingSetFile;
  trainingSetFile.open("trainingset.txt");
  vector<Sample> trainingSet;
  double x1;
  int t1;
  int t2;
  while(trainingSetFile >> x1) {
    trainingSetFile >> t1;
    trainingSetFile >> t2;
    Sample sample;
    sample.x1=x1;
    sample.t1=t1;
    sample.t2=t2;
    trainingSet.push_back(sample);
  }
  trainingSetFile.close();

  // input test set
  ifstream testSetFile;
  testSetFile.open("testset.txt");
  vector<double> testSet;
  double input;
  while(testSetFile >> input)
    testSet.push_back(input);
  testSetFile.close();

  // import weights
  ifstream weightsFile;
  weightsFile.open("weights.txt");
  vector<double> weights;
  double weight;
  while(weightsFile >> weight)
    weights.push_back(weight);
  weightsFile.close();

  // initialize weights in network
  network.weight[0][0] = weights[0];
  network.weight[0][1] = weights[1];
  network.weight[1][0] = weights[2];
  network.weight[1][1] = weights[3];
  network.next->weight[0][0] = weights[4];
  network.next->weight[0][1] = weights[5];
  network.next->weight[1][0] = weights[6];
  network.next->weight[1][2] = weights[7];

  // open file stream
  ofstream file;
  file.open("output.txt");

  for(unsigned iteration = 0; iteration < 41; iteration++) {

    // initialize each delta weight to zero
    network.deltaWeight[0][0] = 0;
    network.deltaWeight[0][1] = 0;
    network.deltaWeight[1][0] = 0;
    network.deltaWeight[1][1] = 0;
    network.next->deltaWeight[0][0] = 0;
    network.next->deltaWeight[0][1] = 0;
    network.next->deltaWeight[1][0] = 0;
    network.next->deltaWeight[1][1] = 0;

    for(unsigned int i = 0; i < trainingSet.size(); i++) {
      // propagate input forward
      network.nodes[0].output = 1;
      network.nodes[1].output = trainingSet[i].x1;
      network.next->nodes[0].input = network.nodes[0].output*network.weight[0][0]+network.nodes[1].output*network.weight[1][0];
      network.next->nodes[1].input = network.nodes[0].output*network.weight[0][1]+network.nodes[1].output*network.weight[1][1];
      network.next->nodes[0].output = computeOutput(network.next->nodes[0].input);
      network.next->nodes[1].output = computeOutput(network.next->nodes[1].input);
      network.next->next->nodes[0].input = network.next->nodes[0].output*network.next->weight[0][0]+network.next->nodes[1].output*network.next->weight[1][0];
      network.next->next->nodes[1].input = network.next->nodes[0].output*network.next->weight[0][1]+network.next->nodes[1].output*network.next->weight[1][1];
      network.next->next->nodes[0].output = computeOutput(network.next->next->nodes[0].input);
      network.next->next->nodes[1].output = computeOutput(network.next->next->nodes[1].input);

      // propagate errors backward
      network.next->next->nodes[0].delta = network.next->next->nodes[0].output*(1 - network.next->next->nodes[0].output)*(trainingSet[i].t1 - network.next->next->nodes[0].output);
      network.next->next->nodes[1].delta = network.next->next->nodes[1].output*(1 - network.next->next->nodes[1].output)*(trainingSet[i].t2 - network.next->next->nodes[1].output);

      network.next->nodes[0].delta = network.next->nodes[0].output*(1 - network.next->nodes[0].output)*(network.next->next->nodes[0].delta*network.next->weight[0][0]+network.next->next->nodes[1].delta*network.next->weight[0][1]);
      network.next->nodes[1].delta = network.next->nodes[1].output*(1 - network.next->nodes[1].output)*(network.next->next->nodes[0].delta*network.next->weight[1][0]+network.next->next->nodes[1].delta*network.next->weight[1][1]);
      network.nodes[0].delta = network.nodes[0].output*(1 - network.nodes[0].output)*(network.next->nodes[0].delta*network.weight[0][0]+network.next->nodes[1].delta*network.weight[0][1]);
      network.nodes[1].delta = network.nodes[1].output*(1 - network.nodes[1].output)*(network.next->nodes[0].delta*network.weight[1][0]+network.next->nodes[1].delta*network.weight[1][1]);

      network.deltaWeight[0][0] += learningRate*network.nodes[0].delta*network.nodes[0].output;
      network.deltaWeight[0][1] += learningRate*network.nodes[1].delta*network.nodes[0].output;
      network.deltaWeight[1][0] += learningRate*network.nodes[0].delta*network.nodes[1].output;
      network.deltaWeight[1][1] += learningRate*network.nodes[1].delta*network.nodes[1].output;
      network.next->deltaWeight[0][0] += learningRate*network.next->nodes[0].delta*network.next->nodes[0].output;
      network.next->deltaWeight[0][1] += learningRate*network.next->nodes[1].delta*network.next->nodes[0].output;
      network.next->deltaWeight[1][0] += learningRate*network.next->nodes[0].delta*network.next->nodes[1].output;
      network.next->deltaWeight[1][1] += learningRate*network.next->nodes[1].delta*network.next->nodes[1].output;
    }

    // update weights in network
    network.weight[0][0] += network.deltaWeight[0][0];
    network.weight[0][1] += network.deltaWeight[0][1];
    network.weight[1][0] += network.deltaWeight[1][0];
    network.weight[1][1] += network.deltaWeight[1][1];
    network.next->weight[0][0] += network.next->deltaWeight[0][0];
    network.next->weight[0][1] += network.next->deltaWeight[0][1];
    network.next->weight[1][0] += network.next->deltaWeight[1][0];
    network.next->weight[1][1] += network.next->deltaWeight[1][1];

    // output weights
    if(iteration > 0 && iteration % 10 == 0) {
      file << "w (the " << iteration << "th iteration)" << endl;
      file << "w01: " << network.weight[0][0] << " w02: " << network.weight[0][1] << " w11: " << network.weight[1][0] << " w12: " << network.weight[1][1] << endl;
      file << "h (the " << iteration << "th iteration)" << endl;
      file << "h01: " << network.next->weight[0][0] << " h02: " << network.next->weight[0][1] << " h11: " << network.next->weight[1][0] << " h12: " << network.next->weight[1][1] << endl << endl;
    }
  }

  file << endl << "Output for test set:" << endl << endl;

  // test the input
  for(unsigned int i = 0; i < testSet.size(); i++) {
    network.nodes[0].output = 1;
    network.nodes[1].output = testSet[i];
    network.next->nodes[0].input = network.nodes[0].output*network.weight[0][0]+network.nodes[1].output*network.weight[1][0];
    network.next->nodes[1].input = network.nodes[0].output*network.weight[0][1]+network.nodes[1].output*network.weight[1][1];
    network.next->nodes[0].output = computeOutput(network.next->nodes[0].input);
    network.next->nodes[1].output = computeOutput(network.next->nodes[1].input);
    network.next->next->nodes[0].input = network.next->nodes[0].output*network.next->weight[0][0]+network.next->nodes[1].output*network.next->weight[1][0];
    network.next->next->nodes[1].input = network.next->nodes[0].output*network.next->weight[0][1]+network.next->nodes[1].output*network.next->weight[1][1];
    network.next->next->nodes[0].output = computeOutput(network.next->next->nodes[0].input);
    network.next->next->nodes[1].output = computeOutput(network.next->next->nodes[1].input);
    file << "x1: " << network.next->next->nodes[0].output << " x2: " << network.next->next->nodes[1].output << endl;
  }

  // close file stream
  file.close();

  // end of program
  return 0;
}
