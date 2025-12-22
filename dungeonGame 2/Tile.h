#include <gui/Icon.h>

class GameTile : public gui::Label 
{
private:
    TileType _type;
    bool _isPlayer;
    bool _isExplored;
    bool _isOnPath;
    bool _isFinalPath;
    
public:
    GameTile(TileType type = TileType::EMPTY)
        : gui::Label(getTileText(type))
        , _type(type)
        , _isPlayer(false)
        , _isExplored(false)
        , _isOnPath(false)
        , _isFinalPath(false)
    {
        setSizeLimits(40, gui::Control::Limit::Fixed, 40, gui::Control::Limit::Fixed);
        updateAppearance();
    }
    
    void setType(TileType type) {
        _type = type;
        updateAppearance();
    }
    
    void setIsPlayer(bool isPlayer) {
        _isPlayer = isPlayer;
        updateAppearance();
    }
    
    void setIsExplored(bool explored) {
        _isExplored = explored;
        updateAppearance();
    }
    
    void setIsOnPath(bool onPath) {
        _isOnPath = onPath;
        updateAppearance();
    }
    
    void setIsFinalPath(bool finalPath) {
        _isFinalPath = finalPath;
        updateAppearance();
    }
    
private:
    const char* getTileText(TileType type) {
        switch(type) {
            case TileType::START: return "S";
            case TileType::EXIT: return "E";
            case TileType::REWARD: return "R";
            case TileType::BANDIT: return "B";
            case TileType::MINE: return "M";
            case TileType::PLAYER: return "P";
            case TileType::EMPTY: 
            default: return " ";
        }
    }
    
    td::ColorID getTileColor(TileType type) {
        switch(type) {
            case TileType::START: return td::ColorID(0x4a6b3d); // Greenish
            case TileType::EXIT: return td::ColorID(0x6a8b4e); // Lighter green
            case TileType::REWARD: return td::ColorID(0xd69c62); // Gold
            case TileType::BANDIT: return td::ColorID(0xc64a2a); // Red
            case TileType::MINE: return td::ColorID(0x8a7a5d); // Brown
            case TileType::PLAYER: return td::ColorID(0x7a9b6e); // Blue-green
            case TileType::EMPTY: 
            default: 
                return _isExplored ? td::ColorID(0x4a4a3d) : 
                       _isOnPath ? td::ColorID(0x5a5a4d) : 
                       _isFinalPath ? td::ColorID(0x8b7a4c) : 
                       td::ColorID(0x22301e);
        }
    }
    
    void updateAppearance() {
        if (_isPlayer) {
            setTitle("P");
            setBackgroundColor(td::ColorID(0x7a9b6e));
            setTextColor(td::ColorID(0xffffff));
        } else {
            setTitle(getTileText(_type));
            setBackgroundColor(getTileColor(_type));
            setTextColor(td::ColorID(0xe8f4d8));
        }
        
        // Add border for certain tile types
        if (_type == TileType::START || _type == TileType::EXIT) {
            setBorderColor(td::ColorID(0xc67a4a));
            setBorderWidth(2);
        } else if (_type == TileType::MINE) {
            setBorderColor(td::ColorID(0x8a7a5d));
            setBorderWidth(1);
        } else {
            setBorderColor(td::ColorID(0x4a6b3d));
            setBorderWidth(1);
        }
    }
};