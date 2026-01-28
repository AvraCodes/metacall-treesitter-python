def greet(name):
    """Public function"""
    print(f"Hello {name}")

def _internal_helper():
    """Private function - should be ignored"""
    pass

def __magic__():
    """Dunder function - should be ignored"""
    pass

class Person:
    """Public class"""
    
    def __init__(self, name):
        """Dunder method - should be ignored"""
        self.name = name
    
    def say_hello(self):
        """Public method"""
        print(self.name)
    
    def _private_method(self):
        """Private method - should be ignored"""
        pass

class _InternalClass:
    """Private class - should be ignored"""
    
    def public_method(self):
        pass

def add(x, y):
    """Another public function"""
    return x + y
