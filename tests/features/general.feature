Feature: General requests and answers from superNET API
    Get peers
    Ping/Pong
    SendMessage/ReceiveMessage

  @general @peers
  Scenario: As a superNET user I want to see peers on my network
    Given superNET is running on the computer
    Then I receive a list of peers with at least two peers

  @general @ping
  Scenario: As a superNET user I want to make a ping and receive an answer
    Given superNET is running on the computer with some peers
    When I send a ping request to one random peer
    Then I receive a pong answer
    # Pong response identification is still needed

  @general @send_message
  Scenario: As a superNET user I want to make a ping and receive an answer
    Given superNET is running on the computer with some peers
    When I send a message to myself with content "Cucumber SuperNET test"
    Then I receive a message to myself with content "Cucumber SuperNET test" in less than 50 seconds