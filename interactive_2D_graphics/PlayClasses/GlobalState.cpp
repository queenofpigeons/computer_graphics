
#include "GlobalState.h"

#include <memory>
#include <algorithm>


std::shared_ptr<TitleMap> readTitleMap(const std::string &title_map_path) {
    std::ifstream input_stream(title_map_path);
    if (!input_stream.is_open()) {
        std::cout << "Unable to open room file " << title_map_path << "\n";
        exit(1);
    }

    std::shared_ptr<TitleMap> title_map = std::make_shared<TitleMap>();
    std::string line;
    int i = 0;
    while (getline(input_stream, line)) { // todo i++
        if (line.length() != h_WINDOW_T_WIDTH * h_MAP_CODE_SIZE) {
            std::cout << title_map_path << " incorrect h_WINDOW_WIDTH "
                      << line.length() << " != " << h_WINDOW_T_WIDTH * h_MAP_CODE_SIZE << "\n";
            exit(1);
        }
        for (int j = 0; j < h_WINDOW_T_WIDTH; ++j) {
            // title_map's items belong to [0,...N]
            (*title_map)[h_WINDOW_T_HEIGHT - i - 1][j] =
                    std::stoi(line.substr(j * h_MAP_CODE_SIZE, h_MAP_CODE_SIZE));
        };
        ++i;
    }
    if (i != h_WINDOW_T_HEIGHT) {
        std::cout << title_map_path << " incorrect h_WINDOW_T_HEIGHT "
                  << i << " != " << h_WINDOW_T_HEIGHT << "\n";
        exit(1);
    }
    return title_map;
};

std::shared_ptr<TransitionsData> readTransitions(const std::string &transitions_path) {
    std::ifstream input_stream(transitions_path);
    if (!input_stream.is_open()) {
        std::cout << "Unable to open room file " << transitions_path << "\n";
        exit(2);
    }

    std::shared_ptr<TransitionsData> transitions_data = std::make_shared<TransitionsData>();
    std::string line;
    if (!getline(input_stream, line)) {
        std::cout << "Transitions incorrect " << transitions_path << "\n";
        exit(2);
    }
    input_stream.close();
    if (line.length() != h_N_TRANSITIONS * h_MAP_CODE_SIZE) {
        std::cout << transitions_path << " incorrect h_N_TRANSITIONS "
                  << line.length() << " != " << h_N_TRANSITIONS * h_MAP_CODE_SIZE << "\n";
        exit(2);
    }
    for (int j = 0; j < h_N_TRANSITIONS; ++j) {
        (*transitions_data)[j] =
                std::stoi(line.substr(j * h_MAP_CODE_SIZE, h_MAP_CODE_SIZE));
    };
    return transitions_data;
};

std::shared_ptr<TransitionsData> getTransitionsPos(
        std::shared_ptr<TransitionsData> &transitions_data,
        std::shared_ptr<TitleMap>& background_map) {
    std::shared_ptr<TransitionsData> transitions_pos = std::make_shared<TransitionsData>(
            TransitionsData{h_WINDOW_T_WIDTH / 2,
                            h_WINDOW_T_HEIGHT / 2,
                            h_WINDOW_T_WIDTH / 2,
                            h_WINDOW_T_HEIGHT / 2});
    if ((*transitions_data)[0] >= 0) {
        for (int j = 0; j < h_WINDOW_T_WIDTH; ++j) {
            if ((*background_map)[h_WINDOW_T_HEIGHT - 1][j] == h_LAVA_CENTRE) {
                (*transitions_pos)[0] = j;
                break;
            }}};
    if ((*transitions_data)[1] >= 0) {
        for (int i = 0; i < h_WINDOW_T_HEIGHT; ++i) {
            if ((*background_map)[i][h_WINDOW_T_WIDTH - 1] == h_LAVA_CENTRE) {
                (*transitions_pos)[1] = i;
                break;
            }}};
    if ((*transitions_data)[2] >= 0) {
        for (int j = 0; j < h_WINDOW_T_WIDTH; ++j) {
            if ((*background_map)[0][j] == h_LAVA_CENTRE) {
                (*transitions_pos)[0] = j;
                break;
            }}};
    if ((*transitions_data)[3] >= 0) {
        for (int i = 0; i < h_WINDOW_T_HEIGHT; ++i) {
            if ((*background_map)[i][0] == h_LAVA_CENTRE) {
                (*transitions_pos)[3] = i;
                break;
            }}};
    return transitions_pos;
}


GlobalState::GlobalState(const std::string &rooms_data_path) {
    for (int i = 0; i < h_N_ROOMS; ++i) {
        std::string single_room_path = rooms_data_path + "/r" + std::to_string(i);
        this->background_map_vector.push_back(
                readTitleMap(single_room_path + "/background_map.txt"));
        this->objects_map_vector.push_back(
                readTitleMap(single_room_path + "/objects_map.txt"));
        this->transitions_data_vector.push_back(
                readTransitions(single_room_path + "/transitions.txt"));
        this->transitions_pos_vector.push_back(getTransitionsPos(transitions_data_vector.back(),
                                                                 background_map_vector.back()));
    }
    this->n_rooms = this->background_map_vector.size();
    this->_ReassigneState(0);
}

void GlobalState::_ReassigneState(int room_number) {
    if ((room_number < 0) || (room_number >= this->n_rooms)) {
        std::cerr << "room_number must to be: 0 <" << room_number << " <= " << this->n_rooms << "\n";
        exit(3);
    }
    this->room_background_map = this->background_map_vector[room_number];
    this->room_objects_map = this->objects_map_vector[room_number];
    this->room_transitions_data = this->transitions_data_vector[room_number];
    this->current_room = room_number;
    this->transition_direction = 0;
    this->bridges_state = {false, false, false, false};
}

Point getNewPlayerPosition(int transition_direction) {
    switch (transition_direction) {
        case 1: return Point{h_WINDOW_WIDTH / 2, h_TEXTURE_SIZE * 3};
        case 2: return Point{h_WINDOW_WIDTH - h_TEXTURE_SIZE * 3, h_WINDOW_HEIGHT / 2};
        case 3: return Point{h_WINDOW_WIDTH / 2, h_WINDOW_HEIGHT - h_TEXTURE_SIZE * 3};
        case 4: return Point{h_TEXTURE_SIZE * 3, h_WINDOW_HEIGHT / 2};
        default: return {h_WINDOW_HEIGHT / 2, h_WINDOW_WIDTH / 2};
    }
};

bool GlobalState::SwitchRoom() {
    if (this->transition_direction <= 0) { return false; }
    int new_room_number = (*this->room_transitions_data)[transition_direction - 1];
    if (new_room_number < 0) {
        std::clog << "From room " << current_room << " address unset room direction " << transition_direction << "\n";
        return false;
    };
    this->init_player_position = getNewPlayerPosition(this->transition_direction);
    _ReassigneState(new_room_number);
    return true;
};

//void GlobalState::_CheckTransitions() {
//    for(std::shared_ptr<TransitionsData> transitions: this->transitions_data_vector){
//        for (int room: *transitions){
//            TransitionsData target_room_t = *(this->transitions_data_vector[room]);
//            if (std::find(target_room_t.begin(), target_room_t.end(), room) == target_room_t.end()){
//                std::cout<<"room "<<room<<"'s transition data has not transition to "<<
//            };
//        }
//    }
//};
