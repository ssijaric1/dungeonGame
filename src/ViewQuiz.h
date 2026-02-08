//
//  ViewQuiz.h
//  Quiz view with questions from AI course
//
#pragma once
#include <gui/View.h>
#include <gui/Label.h>
#include <gui/CheckBox.h>
#include <gui/VerticalLayout.h>
#include <gui/GridLayout.h>
#include <gui/GridComposer.h>
#include <vector>
#include <random>

class ViewQuiz : public gui::View
{
private:
    struct Question
    {
        td::String text;
        td::String options[4];
        int correctAnswer; // 0-3
    };

    std::vector<Question> _questions;
    gui::Label _lblQuestion;
    gui::CheckBox _chkOption1;
    gui::CheckBox _chkOption2;
    gui::CheckBox _chkOption3;
    gui::CheckBox _chkOption4;
    gui::VerticalLayout _mainLayout;

    int _currentCorrectAnswer = 0;
    int _selectedOption = 0;  // Track which option is selected

    void initializeQuestions()
    {
        _questions.reserve(20);

        // Computer Vision Questions
        _questions.push_back({
            "What percentage of the human brain is involved in visual perception?",
            {"30%", "60%", "45%", "80%"},
            1
            });

        _questions.push_back({
            "What does SIFT stand for?",
            {"Scale-Invariant Feature Transform", "Simple Image Feature Tool", "Spatial Intensity Filter Transform", "Scale-Independent Fast Transform"},
            0
            });

        _questions.push_back({
            "In template matching, what does normalized cross-correlation measure?",
            {"Image brightness", "Similarity between template and image patch", "Edge detection quality", "Color distribution"},
            1
            });

        _questions.push_back({
            "What are the three primary additive wavelengths used in color imaging?",
            {"Red, Yellow, Blue", "Red, Green, Blue", "Cyan, Magenta, Yellow", "Red, Orange, Violet"},
            1
            });

        _questions.push_back({
            "What is the main advantage of Gaussian filters being separable?",
            {"Better image quality", "Faster computation (2K vs K² operations)", "Higher resolution", "Better color accuracy"},
            1
            });

        // Neural Networks Questions
        _questions.push_back({
            "Approximately how many neurons are in the human brain?",
            {"10^8 (100 million)", "10^9 (1 billion)", "10^11 (100 billion)", "10^13 (10 trillion)"},
            2
            });

        _questions.push_back({
            "What is the derivative of the sigmoid function ?(x)?",
            {"?(x)", "?(x) * (1 - ?(x))", "1 - ?(x)", "e^(-x)"},
            1
            });

        _questions.push_back({
            "What activation function is commonly used in hidden layers for faster training?",
            {"Sigmoid", "Tanh", "ReLU", "Softmax"},
            2
            });

        _questions.push_back({
            "What does the backpropagation algorithm use to compute gradients?",
            {"Forward pass only", "Chain rule", "Random sampling", "Linear regression"},
            1
            });

        _questions.push_back({
            "For multi-class classification, which combination is recommended?",
            {"Sigmoid + Cross Entropy", "Softmax + Cross Entropy", "ReLU + MSE", "Tanh + Binary Cross Entropy"},
            1
            });

        // Genetic Algorithm Questions
        _questions.push_back({
            "What are the two main genetic operators in GA?",
            {"Selection and Fitness", "Crossover and Mutation", "Initialization and Termination", "Encoding and Decoding"},
            1
            });

        _questions.push_back({
            "What is the typical mutation rate in genetic algorithms?",
            {"0.5-0.9", "0.01-0.1", "0.3-0.5", "0.9-1.0"},
            1
            });

        _questions.push_back({
            "What does elitism guarantee in genetic algorithms?",
            {"Random exploration", "We never lose the best solution", "Faster convergence", "Population diversity"},
            1
            });

        // Swarm Intelligence Questions
        _questions.push_back({
            "In PSO, what does PBest represent?",
            {"Population best", "Personal best position", "Particle baseline", "Previous best"},
            1
            });

        _questions.push_back({
            "What three components influence particle movement in PSO?",
            {"Speed, direction, momentum", "Inertia, cognitive, social", "Position, velocity, acceleration", "Past, present, future"},
            1
            });

        _questions.push_back({
            "What is the main inspiration for Ant Colony Optimization?",
            {"Bird flocking", "Fish schooling", "Ants finding shortest paths using pheromones", "Bee navigation"},
            2
            });

        // Expert Systems Questions
        _questions.push_back({
            "What algorithm does the RETE algorithm use for pattern matching?",
            {"Graph-based inference with alpha and beta networks", "Linear search", "Binary tree search", "Hash table lookup"},
            0
            });

        _questions.push_back({
            "What are the two types of inference in expert systems?",
            {"Static and Dynamic", "Forward and Backward chaining", "Top-down and Bottom-up", "Sequential and Parallel"},
            1
            });

        // Probabilistic Inference Questions
        _questions.push_back({
            "What is Bayes' Rule formula?",
            {"P(A|B) = P(B|A)", "P(A|B) = P(B|A)*P(A)/P(B)", "P(A|B) = P(A)*P(B)", "P(A|B) = P(A)/P(B)"},
            1
            });

        // MDP Questions
        _questions.push_back({
            "In MDP, what does the Bellman equation help find?",
            {"Shortest path", "Optimal policy by relating state values", "Maximum reward", "Transition probabilities"},
            1
            });
    }

    void selectRandomQuestion()
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, _questions.size() - 1);

        int questionIndex = dis(gen);
        const Question& q = _questions[questionIndex];

        _currentCorrectAnswer = q.correctAnswer;
        _lblQuestion.setTitle(q.text);
        _chkOption1.setTitle(q.options[0]);
        _chkOption2.setTitle(q.options[1]);
        _chkOption3.setTitle(q.options[2]);
        _chkOption4.setTitle(q.options[3]);

        // Reset selection - select first option by default
        _selectedOption = 0;
        _chkOption1.setChecked(true, false);
        _chkOption2.setChecked(false, false);
        _chkOption3.setChecked(false, false);
        _chkOption4.setChecked(false, false);
    }

    // Manual radio button behavior
    void onOption1Clicked()
    {
        _selectedOption = 0;
        _chkOption1.setChecked(true);
        _chkOption2.setChecked(false);
        _chkOption3.setChecked(false);
        _chkOption4.setChecked(false);
    }

    void onOption2Clicked()
    {
        _selectedOption = 1;
        _chkOption1.setChecked(false);
        _chkOption2.setChecked(true);
        _chkOption3.setChecked(false);
        _chkOption4.setChecked(false);
    }

    void onOption3Clicked()
    {
        _selectedOption = 2;
        _chkOption1.setChecked(false);
        _chkOption2.setChecked(false);
        _chkOption3.setChecked(true);
        _chkOption4.setChecked(false);
    }

    void onOption4Clicked()
    {
        _selectedOption = 3;
        _chkOption1.setChecked(false);
        _chkOption2.setChecked(false);
        _chkOption3.setChecked(false);
        _chkOption4.setChecked(true);
    }

public:
    ViewQuiz()
        : _lblQuestion("")
        , _chkOption1("Option 1")
        , _chkOption2("Option 2")
        , _chkOption3("Option 3")
        , _chkOption4("Option 4")
        , _mainLayout(6)
    {
        initializeQuestions();
        selectRandomQuestion();

        _lblQuestion.setFont(gui::Font::ID::SystemBold);

        // Set up onClick handlers to make checkboxes act like radio buttons
        _chkOption1.onClick([this]() { onOption1Clicked(); });
        _chkOption2.onClick([this]() { onOption2Clicked(); });
        _chkOption3.onClick([this]() { onOption3Clicked(); });
        _chkOption4.onClick([this]() { onOption4Clicked(); });

        _mainLayout.append(_lblQuestion);
        _mainLayout.appendSpace(10);
        _mainLayout.append(_chkOption1);
        _mainLayout.append(_chkOption2);
        _mainLayout.append(_chkOption3);
        _mainLayout.append(_chkOption4);//
//  ViewQuiz.h
//  Quiz view with questions from AI course
//
#pragma once
#include <gui/View.h>
#include <gui/Label.h>
#include <gui/CheckBox.h>
#include <gui/VerticalLayout.h>
#include <gui/GridLayout.h>
#include <gui/GridComposer.h>
#include <vector>
#include <random>

        class ViewQuiz : public gui::View
        {
        private:
            struct Question
            {
                td::String text;
                td::String options[4];
                int correctAnswer; // 0-3
            };

            std::vector<Question> _questions;
            gui::Label _lblQuestion;
            gui::CheckBox _chkOption1;
            gui::CheckBox _chkOption2;
            gui::CheckBox _chkOption3;
            gui::CheckBox _chkOption4;
            gui::VerticalLayout _mainLayout;

            int _currentCorrectAnswer = 0;
            int _selectedOption = 0;  // Track which option is selected

            void initializeQuestions()
            {
                _questions.reserve(20);

                // Computer Vision Questions
                _questions.push_back({
                    "What percentage of the human brain is involved in visual perception?",
                    {"30%", "60%", "45%", "80%"},
                    1
                    });

                _questions.push_back({
                    "What does SIFT stand for?",
                    {"Scale-Invariant Feature Transform", "Simple Image Feature Tool", "Spatial Intensity Filter Transform", "Scale-Independent Fast Transform"},
                    0
                    });

                _questions.push_back({
                    "In template matching, what does normalized cross-correlation measure?",
                    {"Image brightness", "Similarity between template and image patch", "Edge detection quality", "Color distribution"},
                    1
                    });

                _questions.push_back({
                    "What are the three primary additive wavelengths used in color imaging?",
                    {"Red, Yellow, Blue", "Red, Green, Blue", "Cyan, Magenta, Yellow", "Red, Orange, Violet"},
                    1
                    });

                _questions.push_back({
                    "What is the main advantage of Gaussian filters being separable?",
                    {"Better image quality", "Faster computation (2K vs K² operations)", "Higher resolution", "Better color accuracy"},
                    1
                    });

                // Neural Networks Questions
                _questions.push_back({
                    "Approximately how many neurons are in the human brain?",
                    {"10^8 (100 million)", "10^9 (1 billion)", "10^11 (100 billion)", "10^13 (10 trillion)"},
                    2
                    });

                _questions.push_back({
                    "What is the derivative of the sigmoid function ?(x)?",
                    {"?(x)", "?(x) * (1 - ?(x))", "1 - ?(x)", "e^(-x)"},
                    1
                    });

                _questions.push_back({
                    "What activation function is commonly used in hidden layers for faster training?",
                    {"Sigmoid", "Tanh", "ReLU", "Softmax"},
                    2
                    });

                _questions.push_back({
                    "What does the backpropagation algorithm use to compute gradients?",
                    {"Forward pass only", "Chain rule", "Random sampling", "Linear regression"},
                    1
                    });

                _questions.push_back({
                    "For multi-class classification, which combination is recommended?",
                    {"Sigmoid + Cross Entropy", "Softmax + Cross Entropy", "ReLU + MSE", "Tanh + Binary Cross Entropy"},
                    1
                    });

                // Genetic Algorithm Questions
                _questions.push_back({
                    "What are the two main genetic operators in GA?",
                    {"Selection and Fitness", "Crossover and Mutation", "Initialization and Termination", "Encoding and Decoding"},
                    1
                    });

                _questions.push_back({
                    "What is the typical mutation rate in genetic algorithms?",
                    {"0.5-0.9", "0.01-0.1", "0.3-0.5", "0.9-1.0"},
                    1
                    });

                _questions.push_back({
                    "What does elitism guarantee in genetic algorithms?",
                    {"Random exploration", "We never lose the best solution", "Faster convergence", "Population diversity"},
                    1
                    });

                // Swarm Intelligence Questions
                _questions.push_back({
                    "In PSO, what does PBest represent?",
                    {"Population best", "Personal best position", "Particle baseline", "Previous best"},
                    1
                    });

                _questions.push_back({
                    "What three components influence particle movement in PSO?",
                    {"Speed, direction, momentum", "Inertia, cognitive, social", "Position, velocity, acceleration", "Past, present, future"},
                    1
                    });

                _questions.push_back({
                    "What is the main inspiration for Ant Colony Optimization?",
                    {"Bird flocking", "Fish schooling", "Ants finding shortest paths using pheromones", "Bee navigation"},
                    2
                    });

                // Expert Systems Questions
                _questions.push_back({
                    "What algorithm does the RETE algorithm use for pattern matching?",
                    {"Graph-based inference with alpha and beta networks", "Linear search", "Binary tree search", "Hash table lookup"},
                    0
                    });

                _questions.push_back({
                    "What are the two types of inference in expert systems?",
                    {"Static and Dynamic", "Forward and Backward chaining", "Top-down and Bottom-up", "Sequential and Parallel"},
                    1
                    });

                // Probabilistic Inference Questions
                _questions.push_back({
                    "What is Bayes' Rule formula?",
                    {"P(A|B) = P(B|A)", "P(A|B) = P(B|A)*P(A)/P(B)", "P(A|B) = P(A)*P(B)", "P(A|B) = P(A)/P(B)"},
                    1
                    });

                // MDP Questions
                _questions.push_back({
                    "In MDP, what does the Bellman equation help find?",
                    {"Shortest path", "Optimal policy by relating state values", "Maximum reward", "Transition probabilities"},
                    1
                    });
            }

            void selectRandomQuestion()
            {
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, _questions.size() - 1);

                int questionIndex = dis(gen);
                const Question& q = _questions[questionIndex];

                _currentCorrectAnswer = q.correctAnswer;
                _lblQuestion.setTitle(q.text);
                _chkOption1.setTitle(q.options[0]);
                _chkOption2.setTitle(q.options[1]);
                _chkOption3.setTitle(q.options[2]);
                _chkOption4.setTitle(q.options[3]);

                // Reset selection - select first option by default
                _selectedOption = 0;
                _chkOption1.setChecked(true, false);   // false = don't trigger onClick
                _chkOption2.setChecked(false, false);
                _chkOption3.setChecked(false, false);
                _chkOption4.setChecked(false, false);
            }

            // Manual radio button behavior
            // IMPORTANT: Use sendMessage=false to prevent infinite recursion!
            void onOption1Clicked()
            {
                _selectedOption = 0;
                _chkOption1.setChecked(true, false);   // false = don't trigger onClick
                _chkOption2.setChecked(false, false);
                _chkOption3.setChecked(false, false);
                _chkOption4.setChecked(false, false);
            }

            void onOption2Clicked()
            {
                _selectedOption = 1;
                _chkOption1.setChecked(false, false);
                _chkOption2.setChecked(true,  false);
                _chkOption3.setChecked(false, false);
                _chkOption4.setChecked(false, false);
            }

            void onOption3Clicked()
            {
                _selectedOption = 2;
                _chkOption1.setChecked(false, false);
                _chkOption2.setChecked(false, false);
                _chkOption3.setChecked(true, false);
                _chkOption4.setChecked(false, false);
            }

            void onOption4Clicked()
            {
                _selectedOption = 3;
                _chkOption1.setChecked(false, false);
                _chkOption2.setChecked(false, false);
                _chkOption3.setChecked(false, false);
                _chkOption4.setChecked(true, false);
            }

        public:
            ViewQuiz()
                : _lblQuestion("")
                , _chkOption1("Option 1")
                , _chkOption2("Option 2")
                , _chkOption3("Option 3")
                , _chkOption4("Option 4")
                , _mainLayout(6)
            {
                initializeQuestions();
                selectRandomQuestion();

                _lblQuestion.setFont(gui::Font::ID::SystemBold);

                // Set up onClick handlers to make checkboxes act like radio buttons
                _chkOption1.onClick([this]() { onOption1Clicked(); });
                _chkOption2.onClick([this]() { onOption2Clicked(); });
                _chkOption3.onClick([this]() { onOption3Clicked(); });
                _chkOption4.onClick([this]() { onOption4Clicked(); });

                _mainLayout.append(_lblQuestion);
                _mainLayout.appendSpace(10);
                _mainLayout.append(_chkOption1);
                _mainLayout.append(_chkOption2);
                _mainLayout.append(_chkOption3);
                _mainLayout.append(_chkOption4);

                setLayout(&_mainLayout);
            }

            bool isAnswerCorrect() const
            {
                return _selectedOption == _currentCorrectAnswer;
            }
        };

        setLayout(&_mainLayout);
    }

    bool isAnswerCorrect() const
    {
        return _selectedOption == _currentCorrectAnswer;
    }
};