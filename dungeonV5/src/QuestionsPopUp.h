#pragma once
#include <gui/Dialog.h>
#include <gui/Label.h>
#include <gui/GridLayout.h>
#include <gui/View.h>
#include <vector>
#include <string>
#include <random>
#include <ctime>

struct Question {
    std::string text;
    std::vector<std::string> answers;  
    int correctAnswerIndex;  
};

class DialogLogin : public gui::Dialog
{
protected:
    gui::View _contentView;
    gui::Label _lblQuestion;
    gui::GridLayout _gl;
    int _selectedAnswer;
    int _correctAnswerIndex;
    
    bool onClick(Dialog::Button::ID btnID, gui::Button* pButton) override
    {
        if (btnID == Dialog::Button::ID::OK) {
            _selectedAnswer = 0;
        }
        else if (btnID == Dialog::Button::ID::Apply) {
            _selectedAnswer = 1;
        }
        else if (btnID == Dialog::Button::ID::Cancel) {
            _selectedAnswer = 2;  
        }
        
        showAnswerFeedback();
        return true; 
    }
    
    void showAnswerFeedback() {
        if (_selectedAnswer == _correctAnswerIndex) {
            showAlert("Correct!", "Good job! You answered correctly!");
        } else {
            showAlert("Wrong!", "Incorrect answer. Better luck next time!");
        }
    }
    
static std::vector<Question> getAllQuestions() {
    return {

        {"What percentage of the human brain is involved in visual perception?", {"30%", "60%", "80%"}, 2},
        {"What does SIFT stand for?", {"Scale-Invariant Feature Transform", "Simple Image Feature Tool", "Spatial Intensity Filter Transform"}, 0},
        {"In template matching, what does normalized cross-correlation measure?", {"Image brightness", "Similarity between template and image patch", "Edge detection quality"}, 1},
        {"What are the three primary additive wavelengths used in color imaging?", {"Red, Yellow, Blue", "Red, Green, Blue", "Cyan, Magenta, Yellow"}, 1},
        {"What is the main advantage of Gaussian filters being separable?", {"Better image quality", "Faster computation (2K vs K² operations)", "Higher resolution"}, 0},
        

        {"Approximately how many neurons are in the human brain?", {"10^8 (100 million)", "10^9 (1 billion)", "10^11 (100 billion)"}, 2},
        {"What is the derivative of the sigmoid function σ(x)?", {"σ(x)", "σ(x) * (1 - σ(x))", "1 - σ(x)"}, 1},
        {"What activation function is commonly used in hidden layers for faster training?", {"Sigmoid", "Tanh", "ReLU"}, 2},
        {"What does the backpropagation algorithm use to compute gradients?", {"Forward pass only", "Chain rule", "Random sampling"}, 1},
        {"For multi-class classification, which combination is recommended?", {"Sigmoid + Cross Entropy", "Softmax + Cross Entropy", "ReLU + MSE"}, 0},
        
        {"What are the two main genetic operators in GA?", {"Selection and Fitness", "Crossover and Mutation", "Initialization and Termination"}, 2},
        {"What is the typical mutation rate in genetic algorithms?", {"0.5-0.9", "0.01-0.1", "0.3-0.5"}, 1},
        {"What does elitism guarantee in genetic algorithms?", {"Random exploration", "We never lose the best solution", "Faster convergence"}, 0},
        
        {"In PSO, what does PBest represent?", {"Population best", "Personal best position", "Particle baseline"}, 1},
        {"What three components influence particle movement in PSO?", {"Speed, direction, momentum", "Inertia, cognitive, social", "Position, velocity, acceleration"}, 2},
        {"What is the main inspiration for Ant Colony Optimization?", {"Bird flocking", "Fish schooling", "Ants finding shortest paths using pheromones"}, 0},
        
        {"What algorithm does the RETE algorithm use for pattern matching?", {"Graph-based inference with alpha and beta networks", "Linear search", "Binary tree search"}, 0},
        {"What are the two types of inference in expert systems?", {"Static and Dynamic", "Forward and Backward chaining", "Top-down and Bottom-up"}, 1},
        
        {"What is Bayes' Rule formula?", {"P(A|B) = P(B|A)", "P(A|B) = P(B|A)*P(A)/P(B)", "P(A|B) = P(A)*P(B)"}, 2},
        
        {"In MDP, what does the Bellman equation help find?", {"Shortest path", "Optimal policy by relating state values", "Maximum reward"}, 1}
    };
}

    static Question getRandomQuestion() {
        static std::vector<Question> questions = getAllQuestions();
        static std::mt19937 rng(static_cast<unsigned int>(time(nullptr)));
        static std::uniform_int_distribution<size_t> dist(0, questions.size() - 1);
        
        return questions[dist(rng)];
    }
    
public:
    DialogLogin(gui::Control* pView, const Question& q, td::UINT4 wndID = 0)
    : gui::Dialog(pView, { 
        {gui::Dialog::Button::ID::OK, q.answers[0].c_str()},
        {gui::Dialog::Button::ID::Apply, q.answers[1].c_str()},
        {gui::Dialog::Button::ID::Cancel, q.answers[2].c_str()}
    }, gui::Size(500, 150), wndID)
    , _lblQuestion(q.text.c_str())
    , _gl(1, 1)
    , _selectedAnswer(-1)
    , _correctAnswerIndex(q.correctAnswerIndex)
    {
        setTitle("AI Knowledge Challenge - Answer Required");
        
        _gl.insert(0, 0, _lblQuestion);
        _contentView.setLayout(&_gl);
        setCentralView(&_contentView);
    }
    
    static DialogLogin* createWithRandomQuestion(gui::Control* pView, td::UINT4 wndID = 0) {
        Question q = getRandomQuestion();
        return new DialogLogin(pView, q, wndID);
    }
    
    bool isAnswerCorrect() const {
        return _selectedAnswer == _correctAnswerIndex;
    }
    
    int getSelectedAnswer() const {
        return _selectedAnswer;
    }
};